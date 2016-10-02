#ifndef TLDR_SRC_ELF_ARCH_X86_ELF_HPP_
#define TLDR_SRC_ELF_ARCH_X86_ELF_HPP_

#include "../../elf.hpp"
#include <tldr/raw_module.hpp>

#include <cstdint>
#include <cstring>

namespace tldr {

template <class ElfN, class Relocation>
bool elf_x86_is_group_stop_relocation(const ElfImageR<ElfN> & image,
                                      const Relocation & reloc)
{
	return false;
}

template <class ElfN>
Elf_Addr<ElfN> elf_x86_relocation_addend(const ElfImageR<ElfN> & image,
                                         const Elf_Rel<ElfN> & rel)
{
	switch (ELF_R_TYPE(rel)) {
	case R_386_NONE: case R_386_COPY:
		return 0;
	case R_386_32: case R_386_PC32:
	case R_386_GLOB_DAT: case R_386_JMP_SLOT: case R_386_RELATIVE:
		return image.template load_from<std::uint32_t>(rel.r_offset);
	default:
		throw LoadError("relocation type not supported");
	}
}

template <class ElfN, class Relocation>
bool elf_x86_is_copy_relocation(const ElfImageR<ElfN> & image,
                                const Relocation & reloc)
{
	return ELF_R_TYPE(reloc) == R_386_COPY;
}

template <class ElfN, class Relocation>
void elf_x86_apply_copy_relocation(ElfImageRw<ElfN> & image,
                                   const Relocation & reloc,
                                   const ElfDynamicTable<ElfN> & dyn_table,
                                   const ElfSymbolResolver<ElfN> & resolver)
{
	const auto sym_table = dyn_table.symbol_table();
	const auto mem_dst = image.rva_to_ptr(reloc.r_offset);
	const auto sym_info = sym_table.get_symbol(ELF_R_SYM(reloc));
	const auto sym_value = elf_resolve_relocation_symbol(image, reloc, dyn_table, resolver);
	const auto mem_src = reinterpret_cast<const void *>(sym_value);
	std::memcpy(mem_dst, mem_src, sym_info.st_size);
}

template <class ElfN, class Relocation>
Elf_Addr<ElfN> elf_x86_compute_relocation(const ElfImageR<ElfN> & image,
                                          const Relocation & reloc,
                                          Elf_Addr<ElfN> addend,
                                          const ElfDynamicTable<ElfN> & dyn_table,
                                          const ElfSymbolResolver<ElfN> & resolver)
{
	switch (ELF_R_TYPE(reloc)) {
	case R_386_NONE:
		return addend;
	case R_386_32: {
		const auto sym_value = elf_resolve_relocation_symbol(image, reloc, dyn_table, resolver);
		return sym_value + addend;
	}
	case R_386_PC32: {
		const auto sym_value = elf_resolve_relocation_symbol(image, reloc, dyn_table, resolver);
		return sym_value - reloc.r_offset + addend;
	}
	case R_386_JMP_SLOT: case R_386_GLOB_DAT:
		return elf_resolve_relocation_symbol(image, reloc, dyn_table, resolver);
	case R_386_RELATIVE:
		return image.vbase() + addend;
	default:
		throw LoadError("relocation type not supported");
	}
}

template <class ElfN, class Relocation>
void elf_x86_store_relocation(ElfImageRw<ElfN> & image,
                              const Relocation & reloc,
                              Elf_Addr<ElfN> value)
{
	switch (ELF_R_TYPE(reloc)) {
	case R_386_32: case R_386_PC32:
	case R_386_GLOB_DAT: case R_386_JMP_SLOT: case R_386_RELATIVE:
		return image.template store_at<std::uint32_t>(reloc.r_offset, value);
	default:
		throw LoadError("relocation type not supported");
	}
}

}

#endif
