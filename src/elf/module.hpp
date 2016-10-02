#ifndef TLDR_SRC_ELF_MODULE_HPP_
#define TLDR_SRC_ELF_MODULE_HPP_

#include <tldr/loader.hpp>
#include <tldr/raw_module.hpp>

#include "elf.hpp"
#include "../vmemory.hpp"
#include "arch/x86/elf.hpp"
#include "arch/x86_64/elf.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace tldr {

template <class ElfN>
class ElfModule final : public Module
{
	friend class ElfSymbolResolver<ElfN>;

public:
	static bool is_valid(const void * mem, std::size_t size);

	ElfModule(const void * mem, std::size_t size, const ModuleResolver & resolver);
	virtual ~ElfModule();

	virtual fn_ptr_t get_raw_proc(const std::string & name) const override;
	virtual data_ptr_t get_raw_data(const std::string & name) const override;

private:
	ElfImageRw<ElfN> image_;
	std::vector<std::shared_ptr<Module>> deps_;
};

#ifdef TLDR_HAS_ELF32_SUPPORT
	typedef ElfModule<Elf32> Elf32Module;
#endif

#ifdef TLDR_HAS_ELF64_SUPPORT
	typedef ElfModule<Elf64> Elf64Module;
#endif

template <class ElfN>
bool ElfModule<ElfN>::is_valid(const void * mem, std::size_t size)
{
	if (!ElfImageR<ElfN>::is_valid(mem, size))
		return false;
	ElfImageR<ElfN> elf_image { mem, size };
	return elf_image.type() == ET_DYN
	    && elf_image.is_compatible();
}

template <typename T = void>
T * apply_offset(void * base, std::ptrdiff_t offset)
{
	const auto data = static_cast<char *>(base);
	return static_cast<T *>(data + offset);
}

template <typename T = void>
const T * apply_offset(const void * base, std::ptrdiff_t offset)
{
	const auto data = static_cast<const char *>(base);
	return static_cast<const T *>(data + offset);
}

template <class ElfN>
void elf_map_program_headers(const ElfImageR<ElfN> & image, void * mem)
{
	for (const auto & phdr : image.phdrs()) {
		if (phdr.p_type == PT_LOAD) {
			const auto mem_rva = phdr.p_vaddr - image.vbase();
			assert(mem_rva + phdr.p_memsz <= image.vsize());
			if (phdr.p_filesz > phdr.p_memsz)
				throw LoadError("invalid elf image (p_filesz > p_memsz)");
			const auto mem_src = image.offset_to_ptr(phdr.p_offset);
			const auto mem_dst = apply_offset<>(mem, phdr.p_vaddr);
			std::memcpy(mem_dst, mem_src, phdr.p_filesz);
		}
	}
}

template <class ElfN>
ElfImageRw<ElfN> elf_load_image(const ElfImageR<ElfN> & image)
{
	const auto image_mem = vmem_alloc(image.vsize(), image.vbase());
	try {
		elf_map_program_headers(image, image_mem);
		return { image_mem, image.vsize() };
	} catch (const std::exception & e) {
		vmem_free(image_mem, image.vsize());
		throw;
	}
}

template <class ElfN>
std::vector<std::shared_ptr<Module>>
elf_resolve_imports(const ElfImageR<ElfN> & image,
                    const ModuleResolver & resolver)
{
	std::vector<std::shared_ptr<Module>> imports;
	if (const auto & dyn_table = image.dynamic_table()) {
		const auto & str_table = dyn_table->string_table();
		for (const auto & dyn : dyn_table->entries()) {
			if (dyn.d_tag == DT_NEEDED) {
				const auto mod_name = str_table.get_string(dyn.d_un.d_val);
				const auto mod_handle = resolver.get_module(mod_name);
				if (!mod_handle)
					throw LoadError("module dependency not found");
				imports.emplace_back(std::move(mod_handle));
			}
		}
	}
	return imports;
}

template <class ElfN>
Elf_Addr<ElfN> elf_relocation_addend(const ElfImageR<ElfN> & image,
                                     const Elf_Rela<ElfN> & rela)
{
	return rela.r_addend;
}

template <class ElfN>
Elf_Addr<ElfN> elf_relocation_addend(const ElfImageR<ElfN> & image,
                                     const Elf_Rel<ElfN> & rel)
{
	switch (image.machine()) {
	case EM_386:
		return elf_x86_relocation_addend(image, rel);
	case EM_X86_64:
		return elf_x86_64_relocation_addend(image, rel);
	}
}

template <class ElfN>
Elf_Addr<ElfN> elf_resolve_symbol(const std::string & sym_name,
                                  const Elf_Sym<ElfN> & sym_info,
                                  const ElfSymbolResolver<ElfN> & resolver)
{
	switch (ELF_ST_TYPE(sym_info)) {
	case STT_OBJECT: return resolver.get_data_symbol(sym_name);
	case STT_FUNC: return resolver.get_proc_symbol(sym_name);
	default: return 0;
	}
}

template <class ElfN, class Relocation>
Elf_Addr<ElfN> elf_resolve_relocation_symbol(const ElfImageR<ElfN> & image,
                                             const Relocation & reloc,
                                             const ElfDynamicTable<ElfN> & dyn_table,
                                             const ElfSymbolResolver<ElfN> & resolver)
{
	const auto & sym_table = dyn_table.symbol_table();
	const auto & str_table = dyn_table.string_table();
	const auto sym_info = sym_table.get_symbol(ELF_R_SYM(reloc));
	const auto sym_name = str_table.get_string(sym_info.st_name);
	const auto sym_value = elf_resolve_symbol(sym_name, sym_info, resolver);
	if (!sym_value && ELF_ST_BIND(sym_info) != STB_WEAK)
		throw LoadError("required symbol not found");
	return sym_value;
}

template <typename ElfN, class Relocation>
bool elf_is_group_stop_relocation(const ElfImageR<ElfN> & image,
                                  const Relocation & reloc)
{
	switch (image.machine()) {
	case EM_386:
		return elf_x86_is_group_stop_relocation(image, reloc);
	case EM_X86_64:
		return elf_x86_64_is_group_stop_relocation(image, reloc);
	}
}

template <class ElfN, typename Relocation>
bool elf_is_copy_relocation(const ElfImageR<ElfN> & image,
                            const Relocation & reloc)
{
	switch (image.machine()) {
	case EM_386:
		return elf_x86_is_copy_relocation(image, reloc);
	case EM_X86_64:
		return elf_x86_64_is_copy_relocation(image, reloc);
	}
}

template <class ElfN, typename Relocation>
void elf_apply_copy_relocation(ElfImageRw<ElfN> & image,
                               const Relocation & reloc,
                               const ElfDynamicTable<ElfN> & dyn_table,
                               const ElfSymbolResolver<ElfN> & resolver)
{
	switch (image.machine()) {
	case EM_386:
		return elf_x86_apply_copy_relocation(image, reloc, dyn_table, resolver);
	case EM_X86_64:
		return elf_x86_64_apply_copy_relocation(image, reloc, dyn_table, resolver);
	}
}

template <class ElfN, class Relocation>
Elf_Addr<ElfN> elf_compute_relocation(const ElfImageR<ElfN> & image,
                                      const Relocation & reloc,
                                      Elf_Addr<ElfN> addend,
                                      const ElfDynamicTable<ElfN> & dyn_table,
                                      const ElfSymbolResolver<ElfN> & resolver)
{
	switch (image.machine()) {
	case EM_386:
		return elf_x86_compute_relocation(image, reloc, addend, dyn_table, resolver);
	case EM_X86_64:
		return elf_x86_64_compute_relocation(image, reloc, addend, dyn_table, resolver);
	}
}

template <typename ElfN, class Relocation>
void elf_store_relocation(ElfImageRw<ElfN> & image,
                          const Relocation & reloc,
                          Elf_Addr<ElfN> value)
{
	switch (image.machine()) {
	case EM_386:
		return elf_x86_store_relocation(image, reloc, value);
	case EM_X86_64:
		return elf_x86_64_store_relocation(image, reloc, value);
	}
}

template <class ElfN, class RelocationRange>
void elf_apply_relocation_group(ElfImageRw<ElfN> & image,
                                const RelocationRange & relocs,
                                const ElfDynamicTable<ElfN> & dyn_table,
                                const ElfSymbolResolver<ElfN> & resolver)
{
	const auto enditer = relocs.end();
	for (auto iter = relocs.begin(); iter != enditer;) {
		const auto reloffs = iter->r_offset;
		const auto reladdr = reloffs - image.vbase();
		const auto memptr = image.rva_to_ptr(reladdr);
		auto value = elf_relocation_addend(image, *iter);
		std::decay_t<decltype(*iter)> lastreloc;
		for (; iter != enditer; ++iter) {
			if (iter->r_offset != reloffs) break;
			if (elf_is_group_stop_relocation(image, *iter)) break;
			if (elf_is_copy_relocation(image, *iter)) {
				elf_apply_copy_relocation(image, *iter, dyn_table, resolver);
				continue;
			}
			value = elf_compute_relocation(image, *iter, value, dyn_table, resolver);
			lastreloc = *iter;
		}
		elf_store_relocation(image, lastreloc, value);
	}
}

template <class ElfN>
void elf_apply_image_relocations(ElfImageRw<ElfN> & image,
                                 const ElfDynamicTable<ElfN> & dyn_table,
                                 const ElfSymbolResolver<ElfN> & resolver)
{
	elf_apply_relocation_group(image, dyn_table.rels(), dyn_table, resolver);
	elf_apply_relocation_group(image, dyn_table.relas(), dyn_table, resolver);
	elf_apply_relocation_group(image, dyn_table.plt_rels(), dyn_table, resolver);
	elf_apply_relocation_group(image, dyn_table.plt_relas(), dyn_table, resolver);
}

inline int elf_memory_access_flags(int flags)
{
	int result = MemAccessNone;
	if (flags & PF_R) result |= MemAccessRead;
	if (flags & PF_W) result |= MemAccessWrite;
	if (flags & PF_X) result |= MemAccessExecute;
	return result;
}

template <class ElfN>
void elf_apply_memory_permissions(ElfImageRw<ElfN> & image)
{
	for (const auto & phdr : image.phdrs()) {
		if (phdr.p_type == PT_LOAD) {
			assert(phdr.p_filesz <= phdr.p_memsz);
			const auto mem_addr = elf_align(phdr.p_vaddr, phdr.p_align);
			const auto mem_ptr = image.rva_to_ptr(mem_addr - image.vbase());
			const auto access = elf_memory_access_flags(phdr.p_flags);
			vmem_protect(mem_ptr, phdr.p_memsz, access);
		}
	}
}

template <class ElfN>
void elf_run_image_init(const ElfImageR<ElfN> & image,
                        const ElfDynamicTable<ElfN> & dyn_table)
{
	for (const auto & dyn : dyn_table.entries()) {
		if (dyn.d_tag == DT_INIT) {
			const auto init_ptr = image.rva_to_ptr(dyn.d_un.d_ptr);
			return reinterpret_cast<fn_ptr_t>(init_ptr)();
		}
	}
}

template <class ElfN>
void elf_run_image_entry(const ElfImageR<ElfN> & image)
{
	const auto & ehdr = image.ehdr();
	if (ehdr.e_entry != 0) {
		const auto entry_ptr = image.rva_to_ptr(ehdr.e_entry);
		reinterpret_cast<fn_ptr_t>(entry_ptr)();
	}
}

template <class ElfN>
void elf_run_image_init_array(const ElfImageR<ElfN> & image,
                              const ElfDynamicTable<ElfN> & dyn_table)
{
	for (const auto init_rva : dyn_table.init_array()) {
		if (init_rva == 0 || init_rva == -1) continue;
		const auto init_ptr = image.rva_to_ptr(init_rva);
		reinterpret_cast<fn_ptr_t>(init_ptr)();
	}
}

template <class ElfN>
void elf_run_image_preinit_array(const ElfImageR<ElfN> & image,
                              const ElfDynamicTable<ElfN> & dyn_table)
{
	for (const auto preinit_rva : dyn_table.preinit_array()) {
		if (preinit_rva == 0 || preinit_rva == -1) continue;
		const auto preinit_ptr = image.rva_to_ptr(preinit_rva);
		reinterpret_cast<fn_ptr_t>(preinit_ptr)();
	}
}

template <class ElfN>
void elf_initialize_image(const ElfImageR<ElfN> & image)
{
	if (const auto dyn_table = image.dynamic_table()) {
		elf_run_image_init(image, *dyn_table);
		elf_run_image_init_array(image, *dyn_table);
		elf_run_image_preinit_array(image, *dyn_table);
	}
	elf_run_image_entry(image);
}

template <class ElfN>
ElfModule<ElfN>::ElfModule(const void * mem, std::size_t size,
                           const ModuleResolver & resolver)
	: image_ { elf_load_image<ElfN>({ mem, size }) }
	, deps_ { elf_resolve_imports(image_, resolver) }
{
	if (const auto & dyn_table = image_.dynamic_table()) {
		const ElfSymbolResolver<ElfN> sym_resolver { *this };
		elf_apply_image_relocations(image_, *dyn_table, sym_resolver);
	}
	elf_apply_memory_permissions(image_);
	elf_initialize_image(image_);
}

template <class ElfN>
void elf_run_image_fini_array(const ElfImageR<ElfN> & image,
                              const ElfDynamicTable<ElfN> & dyn_table)
{
	for (const auto fini_rva : dyn_table.fini_array()) {
		if (fini_rva == 0 || fini_rva == -1) continue;
		const auto fini_ptr = image.rva_to_ptr(fini_rva);
		reinterpret_cast<fn_ptr_t>(fini_ptr)();
	}
}

template <class ElfN>
void elf_run_image_fini(const ElfImageR<ElfN> & image,
                        const ElfDynamicTable<ElfN> & dyn_table)
{
	for (const auto & dyn : dyn_table.entries()) {
		if (dyn.d_tag == DT_FINI) {
			const auto fini_ptr = image.rva_to_ptr(dyn.d_un.d_ptr);
			return reinterpret_cast<fn_ptr_t>(fini_ptr)();
		}
	}
}

template <class ElfN>
void elf_unload_image(ElfImageRw<ElfN> & image)
{
	if (const auto dyn_table = image.dynamic_table()) {
		elf_run_image_fini_array(image, *dyn_table);
		elf_run_image_fini(image, *dyn_table);
	}
	vmem_free(image.rva_to_ptr(0), image.vsize());
}

template <class ElfN>
ElfModule<ElfN>::~ElfModule()
{
	elf_unload_image(image_);
}

template <class ElfN>
bool elf_is_public_symbol(const Elf_Sym<ElfN> & sym)
{
	const auto visibility = ELF_ST_VISIBILITY(sym);
	return visibility == STV_DEFAULT
	    || visibility == STV_PROTECTED;
}

template <class ElfN>
std::uintptr_t elf_find_symbol(const ElfImageR<ElfN> & image,
                               const std::string & sym_name)
{
	const auto & dyn_table = image.dynamic_table();
	if (!dyn_table) return 0;
	const auto & hash_table = dyn_table->hash_table();
	const auto & sym_table = dyn_table->symbol_table();
	const auto & str_table = dyn_table->string_table();
	const auto sym = hash_table.find_symbol(sym_table, str_table, sym_name);
	if (!sym || !elf_is_public_symbol<ElfN>(*sym)) return 0;
	const auto value = image.rva_to_ptr(sym->st_value);
	return reinterpret_cast<std::uintptr_t>(value);
}

template <class ElfN>
fn_ptr_t ElfModule<ElfN>::get_raw_proc(const std::string & name) const
{
	return reinterpret_cast<fn_ptr_t>(elf_find_symbol(image_, name));
}

template <class ElfN>
data_ptr_t ElfModule<ElfN>::get_raw_data(const std::string & name) const
{
	return reinterpret_cast<data_ptr_t>(elf_find_symbol(image_, name));
}

}

#endif
