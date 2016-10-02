#ifndef TLDR_SRC_ELF_ELF_HPP_
#define TLDR_SRC_ELF_ELF_HPP_

#include <elf.h>
#include "hash.hpp"
#include "endian.hpp"

#include <boost/optional.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <cstdint>
#include <limits>

namespace tldr {

struct Elf32 {
	typedef Elf32_Addr Addr;
	typedef Elf32_Word Word;
	typedef Elf32_Ehdr Ehdr;
	typedef Elf32_Phdr Phdr;
	typedef Elf32_Shdr Shdr;
	typedef Elf32_Dyn  Dyn;
	typedef Elf32_Sym  Sym;
	typedef Elf32_Rel  Rel;
	typedef Elf32_Rela Rela;
	typedef Elf32_Hash Hash;
	typedef Elf32_GnuHash GnuHash;
};

struct Elf64 {
	typedef Elf64_Addr Addr;
	typedef Elf64_Word Word;
	typedef Elf64_Ehdr Ehdr;
	typedef Elf64_Phdr Phdr;
	typedef Elf64_Shdr Shdr;
	typedef Elf64_Dyn  Dyn;
	typedef Elf64_Sym  Sym;
	typedef Elf64_Rel  Rel;
	typedef Elf64_Rela Rela;
	typedef Elf64_Hash Hash;
	typedef Elf64_GnuHash GnuHash;
};

template <class ElfN> using Elf_Addr = typename ElfN::Addr;
template <class ElfN> using Elf_Word = typename ElfN::Word;
template <class ElfN> using Elf_Ehdr = typename ElfN::Ehdr;
template <class ElfN> using Elf_Phdr = typename ElfN::Phdr;
template <class ElfN> using Elf_Shdr = typename ElfN::Shdr;
template <class ElfN> using Elf_Dyn  = typename ElfN::Dyn;
template <class ElfN> using Elf_Sym  = typename ElfN::Sym;
template <class ElfN> using Elf_Rel  = typename ElfN::Rel;
template <class ElfN> using Elf_Rela = typename ElfN::Rela;
template <class ElfN> using Elf_Hash = typename ElfN::Hash;
template <class ElfN> using Elf_GnuHash = typename ElfN::GnuHash;

template <class ElfN> class ElfModule;
template <class ElfN> class ElfDynamicTable;
template <class ElfN, typename T> class ElfObjectRange;

template <class ElfN, typename VoidP>
class ElfImage
{
	template <class, typename>
	friend class ElfImage;

public:
	using phdr_range = ElfObjectRange<ElfN, Elf_Phdr<ElfN>>;
	using shdr_range = ElfObjectRange<ElfN, Elf_Shdr<ElfN>>;

	static bool is_valid(const void * mem, std::size_t size);

	ElfImage(VoidP mem, std::size_t size);

	const Elf_Ehdr<ElfN> & ehdr() const;
	unsigned char machine() const;
	unsigned char type() const;
	bool is_compatible() const;

	std::uintptr_t vbase() const;
	std::size_t vsize() const;

	phdr_range phdrs() const;
	shdr_range shdrs() const;

	boost::optional<ElfDynamicTable<ElfN>> dynamic_table() const;

	template <typename T>
	T load_from(std::ptrdiff_t offset) const;

	const void * offset_to_ptr(std::uintptr_t offset) const;
	const void * rva_to_ptr(std::uintptr_t reladdr) const;

private:
	VoidP mem_;
	std::size_t size_;
	std::uintptr_t vbase_;
	std::size_t vsize_;
	Elf_Ehdr<ElfN> ehdr_;
};

template <class ElfN>
using ElfImageR = ElfImage<ElfN, const void *>;

template <class ElfN>
class ElfImage<ElfN, void *> : public ElfImageR<ElfN>
{
public:
	ElfImage(void * mem, std::size_t size);

	template <typename T>
	void store_at(std::ptrdiff_t offset, const T & value);

	using ElfImageR<ElfN>::offset_to_ptr;
	void * offset_to_ptr(std::uintptr_t offset);

	using ElfImageR<ElfN>::rva_to_ptr;
	void * rva_to_ptr(std::uintptr_t reladdr);
};

template <class ElfN>
using ElfImageRw = ElfImage<ElfN, void *>;

template <class ElfN>
class ElfSymbolResolver
{
public:
	explicit ElfSymbolResolver(const ElfModule<ElfN> & module);

	Elf_Addr<ElfN> get_data_symbol(const std::string & name) const;
	Elf_Addr<ElfN> get_proc_symbol(const std::string & name) const;

private:
	template <typename Fn>
	Elf_Addr<ElfN> get_symbol_each(Fn && try_resolve) const;

private:
	const ElfModule<ElfN> & source_;
};

template <class ElfN>
class ElfStringTable
{
public:
	ElfStringTable(const ElfImageR<ElfN> & image, std::uintptr_t reladdr,
	               std::size_t size);

	const char * get_string(std::uintptr_t index) const;

private:
	const ElfImageR<ElfN> * image_;
	std::uintptr_t reladdr_;
	std::size_t size_;
};

template <class ElfN>
class ElfSymbolTable
{
public:
	ElfSymbolTable(const ElfImageR<ElfN> & image, std::uintptr_t reladdr,
	               std::size_t entsize);

	Elf_Sym<ElfN> get_symbol(std::uintptr_t index) const;

private:
	const ElfImageR<ElfN> * image_;
	std::uintptr_t reladdr_;
	std::size_t entsize_;
};

template <class ElfN>
class ElfHashTable
{
public:
	virtual ~ElfHashTable();
	virtual boost::optional<Elf_Sym<ElfN>>
		find_symbol(const ElfSymbolTable<ElfN> & sym_table,
		            const ElfStringTable<ElfN> & str_table,
		            const std::string & sym_name) const = 0;
};

template <class ElfN>
class ElfDynamicTable
{
public:
	using dyn_range = ElfObjectRange<ElfN, Elf_Dyn<ElfN>>;
	using rel_range = ElfObjectRange<ElfN, Elf_Rel<ElfN>>;
	using rela_range = ElfObjectRange<ElfN, Elf_Rela<ElfN>>;
	using addr_range = ElfObjectRange<ElfN, Elf_Addr<ElfN>>;

	ElfDynamicTable(const ElfImageR<ElfN> & image, const Elf_Phdr<ElfN> & dyn_phdr);

	dyn_range entries() const;

	rel_range rels() const;
	rela_range relas() const;

	rel_range plt_rels() const;
	rela_range plt_relas() const;

	addr_range init_array() const;
	addr_range fini_array() const;
	addr_range preinit_array() const;

	ElfStringTable<ElfN> string_table() const;
	ElfSymbolTable<ElfN> symbol_table() const;
	const ElfHashTable<ElfN> & hash_table() const;

private:
	const ElfImageR<ElfN> * image_;
	std::uintptr_t dynaddr_;
	unsigned int entries_;
	std::unique_ptr<ElfHashTable<ElfN>> hash_table_;
};

template <class ElfN>
class ElfLegacyHashTable : public ElfHashTable<ElfN>
{
public:
	ElfLegacyHashTable(const ElfImageR<ElfN> & image, std::uintptr_t reladdr);

	virtual boost::optional<Elf_Sym<ElfN>>
		find_symbol(const ElfSymbolTable<ElfN> & sym_table,
		            const ElfStringTable<ElfN> & str_table,
		            const std::string & sym_name) const override;

private:
	const ElfImageR<ElfN> * image_;
	std::uintptr_t reladdr_;
	Elf_Hash<ElfN> table_;
};

template <class ElfN>
class ElfGnuHashTable : public ElfHashTable<ElfN>
{
public:
	ElfGnuHashTable(const ElfImageR<ElfN> & image, std::uintptr_t reladdr);

	virtual boost::optional<Elf_Sym<ElfN>>
		find_symbol(const ElfSymbolTable<ElfN> & sym_table,
		            const ElfStringTable<ElfN> & str_table,
		            const std::string & sym_name) const override;

private:
	const ElfImageR<ElfN> * image_;
	std::uintptr_t reladdr_;
	Elf_GnuHash<ElfN> table_;
	std::uintptr_t bitmasks_rva_;
	std::uintptr_t buckets_rva_;
	std::uintptr_t chains_rva_;
};

template <class ElfN, typename T>
class ElfObjectIterator : public boost::iterator_facade <
	ElfObjectIterator<ElfN, T>, const T, std::input_iterator_tag
>
{
	friend class ElfObjectRange<ElfN, T>;
	friend class boost::iterator_core_access;

public:
	ElfObjectIterator();

private:
	ElfObjectIterator(ElfObjectRange<ElfN, T> const & range,
	                  std::uintptr_t reladdr);

	void increment();
	const T & dereference() const;
	bool equal(const ElfObjectIterator & other) const;

private:
	const ElfObjectRange<ElfN, T> * range_;
	std::uintptr_t reladdr_;
};

template <class ElfN, typename T>
class ElfObjectRange
{
	friend class ElfObjectIterator<ElfN, T>;

public:
	using iterator = ElfObjectIterator<ElfN, T>;
	using const_iterator = ElfObjectIterator<ElfN, T>;

	ElfObjectRange();
	ElfObjectRange(const ElfImageR<ElfN> & image, std::uintptr_t base,
	               std::size_t entsize, unsigned int count);

	iterator begin() const;
	iterator end() const;

private:
	const ElfImageR<ElfN> * image_;
	std::size_t entsize_;
	std::uintptr_t begin_;
	std::uintptr_t end_;
	mutable T value_;
};

template <typename T> const unsigned char ELFCLASS;
template <> const unsigned char ELFCLASS<Elf32> = ELFCLASS32;
template <> const unsigned char ELFCLASS<Elf64> = ELFCLASS64;

template <class ElfN, typename VoidP>
bool ElfImage<ElfN, VoidP>::is_valid(const void * mem, std::size_t size)
{
	if (size < sizeof(Elf_Ehdr<ElfN>)) return false;
	const auto e_ident = static_cast<const char *>(mem);
	return e_ident[EI_MAG0]  == ELFMAG0
	    && e_ident[EI_MAG1]  == ELFMAG1
	    && e_ident[EI_MAG2]  == ELFMAG2
	    && e_ident[EI_MAG3]  == ELFMAG3
	    && e_ident[EI_CLASS] == ELFCLASS<ElfN>;
}

template <typename T, typename Align>
constexpr T elf_align(T value, Align align)
{
	return (value + align - 1) & ~(align - 1);
}

template <class ElfN, typename VoidP>
ElfImage<ElfN, VoidP>::ElfImage(VoidP mem, std::size_t size)
	: mem_ { mem }, size_ { size }, vbase_ { UINTPTR_MAX }, vsize_ { 0 }
{
	switch (static_cast<const char *>(mem)[EI_DATA]) {
	case ELFDATA2LSB: le_read(mem, size, ehdr_); break;
	case ELFDATA2MSB: be_read(mem, size, ehdr_); break;
	default: throw LoadError("invalid elf image (EI_DATA)");
	}

	for (const auto & phdr : phdrs()) {
		if (phdr.p_type == PT_LOAD) {
			if (phdr.p_vaddr < vbase_) vbase_ = phdr.p_vaddr;
			vsize_ += elf_align(phdr.p_memsz, phdr.p_align);
		}
	}
}

template <class ElfN, typename VoidP>
const Elf_Ehdr<ElfN> & ElfImage<ElfN, VoidP>::ehdr() const
{
	return ehdr_;
}

template <class ElfN, typename VoidP>
unsigned char ElfImage<ElfN, VoidP>::machine() const
{
	return ehdr_.e_machine;
}

template <class ElfN, typename VoidP>
unsigned char ElfImage<ElfN, VoidP>::type() const
{
	return ehdr_.e_type;
}

template <class ElfN, typename VoidP>
bool ElfImage<ElfN, VoidP>::is_compatible() const
{
#if defined(__x86_64) || defined(_M_AMD64)
	return ehdr_.e_machine == EM_X86_64;
#elif defined(__i386__) || defined(_M_IX86)
	return ehdr_.e_machine == EM_386;
#endif
}

template <class ElfN, typename VoidP>
auto ElfImage<ElfN, VoidP>::phdrs() const -> phdr_range
{
	return { *this, ehdr_.e_phoff, ehdr_.e_phentsize, ehdr_.e_phnum };
}

template <class ElfN, typename VoidP>
auto ElfImage<ElfN, VoidP>::shdrs() const -> shdr_range
{
	return { *this, ehdr_.e_shoff, ehdr_.e_shentsize, ehdr_.e_shnum };
}

template <class ElfN, typename VoidP>
boost::optional<ElfDynamicTable<ElfN>> ElfImage<ElfN, VoidP>::dynamic_table() const
{
	for (const auto & phdr : phdrs()) {
		if (phdr.p_type == PT_DYNAMIC)
			return ElfDynamicTable<ElfN>(*this, phdr);
	}
	return boost::none;
}

template <class ElfN, typename VoidP>
std::uintptr_t ElfImage<ElfN, VoidP>::vbase() const
{
	return vbase_;
}

template <class ElfN, typename VoidP>
std::size_t ElfImage<ElfN, VoidP>::vsize() const
{
	return vsize_;
}

template <class ElfN, typename VoidP> template <typename T>
T ElfImage<ElfN, VoidP>::load_from(std::ptrdiff_t offset) const
{
	if (offset < 0 || offset > size_)
		throw std::out_of_range("invalid offset");

	T value;
	const auto data = offset_to_ptr(offset);
	switch (ehdr_.e_ident[EI_DATA]) {
	case ELFDATA2LSB: le_read(data, size_ - offset, value); return value;
	case ELFDATA2MSB: be_read(data, size_ - offset, value); return value;
	}
}

template <class ElfN, typename VoidP>
const void * ElfImage<ElfN, VoidP>::offset_to_ptr(std::uintptr_t offset) const
{
	return static_cast<const char *>(mem_) + offset;
}

template <class ElfN, typename VoidP>
const void * ElfImage<ElfN, VoidP>::rva_to_ptr(std::uintptr_t reladdr) const
{
	return static_cast<const char *>(mem_) + reladdr;
}

template <class ElfN>
ElfImageRw<ElfN>::ElfImage(void * mem, std::size_t size)
	: ElfImageR<ElfN> { mem, size } {}

template <class ElfN> template <typename T>
void ElfImageRw<ElfN>::store_at(std::ptrdiff_t offset, const T & value)
{
	if (offset < 0 || offset > this->size_)
		throw std::out_of_range("invalid offset");

	const auto data = offset_to_ptr(offset);
	switch (this->ehdr_.e_ident[EI_DATA]) {
	case ELFDATA2LSB: le_write(value, data, this->size_ - offset); break;
	case ELFDATA2MSB: be_write(value, data, this->size_ - offset); break;
	}
}

template <class ElfN>
void * ElfImageRw<ElfN>::offset_to_ptr(std::uintptr_t offset)
{
	return const_cast<void *>(ElfImageR<ElfN>::offset_to_ptr(offset));
}

template <class ElfN>
void * ElfImageRw<ElfN>::rva_to_ptr(std::uintptr_t reladdr)
{
	return const_cast<void *>(ElfImageR<ElfN>::rva_to_ptr(reladdr));
}

template <class ElfN>
ElfSymbolResolver<ElfN>::ElfSymbolResolver(const ElfModule<ElfN> & module)
	: source_ { module } {}

template <class ElfN>
Elf_Addr<ElfN> ElfSymbolResolver<ElfN>::get_data_symbol(const std::string & name) const
{
	return get_symbol_each([&] (const auto & module) {
		return module.get_raw_data(name);
	});
}

template <class ElfN>
Elf_Addr<ElfN> ElfSymbolResolver<ElfN>::get_proc_symbol(const std::string & name) const
{
	return get_symbol_each([&] (const auto & module) {
		return module.get_raw_proc(name);
	});
}

template <class ElfN> template <typename Fn>
Elf_Addr<ElfN> ElfSymbolResolver<ElfN>::get_symbol_each(Fn && try_resolve) const
{
	auto sym_value = try_resolve(source_);
	if (!sym_value) {
		for (const auto module : source_.deps_) {
			if (sym_value = try_resolve(*module))
				break;
		}
	}
	return reinterpret_cast<std::uintptr_t>(sym_value);
}

template <class ElfN>
ElfStringTable<ElfN>::ElfStringTable(const ElfImageR<ElfN> & image,
                                     std::uintptr_t reladdr, std::size_t size)
	: image_ { &image }, reladdr_ { reladdr }, size_ { size } {}

template <class ElfN>
const char * ElfStringTable<ElfN>::get_string(std::uintptr_t index) const
{
	if (index > size_) throw std::out_of_range("invalid strtab index");
	return static_cast<const char *>(image_->rva_to_ptr(reladdr_ + index));
}

template <class ElfN>
ElfSymbolTable<ElfN>::ElfSymbolTable(const ElfImageR<ElfN> & image,
                                     std::uintptr_t reladdr, std::size_t entsize)
	: image_ { &image }, reladdr_ { reladdr }, entsize_ { entsize } {}

template <class ElfN>
Elf_Sym<ElfN> ElfSymbolTable<ElfN>::get_symbol(std::uintptr_t index) const
{
	return image_->template load_from<Elf_Sym<ElfN>>(reladdr_ + index * entsize_);
}

template <class ElfN>
ElfDynamicTable<ElfN>::ElfDynamicTable(const ElfImageR<ElfN> & image,
                                       const Elf_Phdr<ElfN> & dyn_phdr)
	: image_ { &image }, dynaddr_ ( dyn_phdr.p_vaddr - image.vbase() )
	, entries_ ( dyn_phdr.p_memsz / sizeof(Elf_Dyn<ElfN>) )
	, hash_table_ { [this] () -> ElfHashTable<ElfN> * {
		for (const auto & dyn : entries()) {
			switch (dyn.d_tag) {
			case DT_HASH: {
				const auto reladdr = dyn.d_un.d_ptr - image_->vbase();
				return new ElfLegacyHashTable<ElfN>(*image_, reladdr);
			}
			case DT_GNU_HASH: {
				const auto reladdr = dyn.d_un.d_ptr - image_->vbase();
				return new ElfGnuHashTable<ElfN>(*image_, reladdr);
			} }
		}
		return nullptr;
	}() }
{
	if (dyn_phdr.p_type != PT_DYNAMIC)
		throw std::logic_error("invalid phdr (!PT_DYNAMIC)");

	if (!hash_table_)
		throw std::runtime_error("invalid elf image (!DT_HASH)");
}

template <class ElfN>
auto ElfDynamicTable<ElfN>::entries() const -> dyn_range
{
	return { *image_, dynaddr_, sizeof(Elf_Dyn<ElfN>), entries_ };
}

template <class ElfN>
ElfStringTable<ElfN> ElfDynamicTable<ElfN>::string_table() const
{
	std::size_t strsz = 0;
	std::uintptr_t strtab = 0;
	for (const auto & dyn : entries()) {
		switch (dyn.d_tag) {
		case DT_STRSZ: strsz = dyn.d_un.d_val; break;
		case DT_STRTAB: strtab = dyn.d_un.d_ptr; break;
		}
	}
	return { *image_, strtab - image_->vbase(), strsz };
}

template <class ElfN>
ElfSymbolTable<ElfN> ElfDynamicTable<ElfN>::symbol_table() const
{
	std::size_t syment = 0;
	std::uintptr_t symtab = 0;
	for (const auto & dyn : entries()) {
		switch (dyn.d_tag) {
		case DT_SYMTAB: symtab = dyn.d_un.d_ptr; break;
		case DT_SYMENT: syment = dyn.d_un.d_val; break;
		}
	}
	return { *image_, symtab - image_->vbase(), syment };
}

template <class ElfN>
const ElfHashTable<ElfN> & ElfDynamicTable<ElfN>::hash_table() const
{
	return *hash_table_;
}

template <class ElfN>
auto ElfDynamicTable<ElfN>::rels() const -> rel_range
{
	std::uintptr_t reladdr = 0;
	std::size_t relsz = 0, entsize = 0;
	for (const auto & dyn : entries()) {
		switch (dyn.d_tag) {
		case DT_REL: reladdr = dyn.d_un.d_ptr; break;
		case DT_RELSZ: relsz = dyn.d_un.d_val; break;
		case DT_RELENT: entsize = dyn.d_un.d_val; break;
		}
	}
	const unsigned int relcount = entsize ? relsz / entsize : 0;
	return { *image_, reladdr - image_->vbase(), entsize, relcount };
}

template <class ElfN>
auto ElfDynamicTable<ElfN>::relas() const -> rela_range
{
	std::uintptr_t reladdr = 0;
	std::size_t relasz = 0, entsize = 0;
	for (const auto & dyn : entries()) {
		switch (dyn.d_tag) {
		case DT_RELA: reladdr = dyn.d_un.d_ptr; break;
		case DT_RELASZ: relasz = dyn.d_un.d_val; break;
		case DT_RELAENT: entsize = dyn.d_un.d_val; break;
		}
	}
	const unsigned int relcount = entsize ? relasz / entsize : 0;
	return { *image_, reladdr - image_->vbase(), entsize, relcount };
}

template <class ElfN>
auto ElfDynamicTable<ElfN>::plt_rels() const -> rel_range
{
	unsigned char reltype = 0;
	std::uintptr_t reladdr = 0;
	std::size_t relsz = 0, entsize = 0;
	for (const auto & dyn : entries()) {
		switch (dyn.d_tag) {
		case DT_JMPREL: reladdr = dyn.d_un.d_ptr; break;
		case DT_PLTREL: reltype = dyn.d_un.d_val; break;
		case DT_RELENT: entsize = dyn.d_un.d_val; break;
		case DT_PLTRELSZ: relsz = dyn.d_un.d_val; break;
		}
	}
	const unsigned int relcount = entsize ? relsz / entsize : 0;
	return reltype == DT_REL
		? rel_range(*image_, reladdr - image_->vbase(), entsize, relcount)
		: rel_range();
}

template <class ElfN>
auto ElfDynamicTable<ElfN>::plt_relas() const -> rela_range
{
	unsigned char reltype = 0;
	std::uintptr_t reladdr = 0;
	std::size_t relasz = 0, entsize = 0;
	for (const auto & dyn : entries()) {
		switch (dyn.d_tag) {
		case DT_JMPREL: reladdr = dyn.d_un.d_ptr; break;
		case DT_PLTREL: reltype = dyn.d_un.d_val; break;
		case DT_RELAENT: entsize = dyn.d_un.d_val; break;
		case DT_PLTRELSZ: relasz = dyn.d_un.d_val; break;
		}
	}
	const unsigned int relcount = entsize ? relasz / entsize : 0;
	return reltype == DT_RELA
		? rela_range(*image_, reladdr - image_->vbase(), entsize, relcount)
		: rela_range();
}

template <class ElfN>
auto ElfDynamicTable<ElfN>::init_array() const -> addr_range
{
	std::size_t arraysz = 0;
	std::uintptr_t reladdr = 0;
	for (const auto & dyn : entries()) {
		switch (dyn.d_tag) {
		case DT_INIT_ARRAY: reladdr = dyn.d_un.d_ptr; break;
		case DT_INIT_ARRAYSZ: arraysz = dyn.d_un.d_val; break;
		}
	}
	const unsigned int count = arraysz / sizeof(Elf_Addr<ElfN>);
	return { *image_, reladdr - image_->vbase(), arraysz, count };
}

template <class ElfN>
auto ElfDynamicTable<ElfN>::preinit_array() const -> addr_range
{
	std::size_t arraysz = 0;
	std::uintptr_t reladdr = 0;
	for (const auto & dyn : entries()) {
		switch (dyn.d_tag) {
		case DT_PREINIT_ARRAY: reladdr = dyn.d_un.d_ptr; break;
		case DT_PREINIT_ARRAYSZ: arraysz = dyn.d_un.d_val; break;
		}
	}
	const unsigned int count = arraysz / sizeof(Elf_Addr<ElfN>);
	return { *image_, reladdr - image_->vbase(), arraysz, count };
}

template <class ElfN>
auto ElfDynamicTable<ElfN>::fini_array() const -> addr_range
{
	std::size_t arraysz = 0;
	std::uintptr_t reladdr = 0;
	for (const auto & dyn : entries()) {
		switch (dyn.d_tag) {
		case DT_FINI_ARRAY: reladdr = dyn.d_un.d_ptr; break;
		case DT_FINI_ARRAYSZ: arraysz = dyn.d_un.d_val; break;
		}
	}
	const unsigned int count = arraysz / sizeof(Elf_Addr<ElfN>);
	return { *image_, reladdr - image_->vbase(), arraysz, count };
}

template <class ElfN>
ElfHashTable<ElfN>::~ElfHashTable() = default;

template <class ElfN>
ElfLegacyHashTable<ElfN>::ElfLegacyHashTable(const ElfImageR<ElfN> & image,
                                             std::uintptr_t reladdr)
	: image_ { &image }, reladdr_ { reladdr }
	, table_ { image.template load_from<Elf_Hash<ElfN>>(reladdr) } {}

/*
	hn = elf_hash(sym_name) % nbuckets;
	for (ndx = hash[ hn ]; ndx; ndx = chain[ ndx ]) {
		symbol = sym_tab + ndx;
		if (strcmp(sym_name, str_tab + symbol->st_name) == 0)
			return (load_addr + symbol->st_value);
	}
 */

template <class ElfN>
boost::optional<Elf_Sym<ElfN>>
ElfLegacyHashTable<ElfN>::find_symbol(const ElfSymbolTable<ElfN> & sym_table,
                                      const ElfStringTable<ElfN> & str_table,
                                      const std::string & sym_name) const
{
	const auto sym_hash = elf_hash(sym_name.c_str());

	return boost::none;
}

template <class ElfN>
ElfGnuHashTable<ElfN>::ElfGnuHashTable(const ElfImageR<ElfN> & image,
                                       std::uintptr_t reladdr)
	: image_ { &image }, reladdr_ { reladdr }
	, table_ { image.template load_from<Elf_GnuHash<ElfN>>(reladdr) }
	, bitmasks_rva_ { reladdr + sizeof(Elf_GnuHash<ElfN>) }
	, buckets_rva_ { bitmasks_rva_ + table_.maskwords * sizeof(Elf_Addr<ElfN>) }
	, chains_rva_ { buckets_rva_ + table_.nbuckets * sizeof(Elf_Word<ElfN>) } {}

template <class ElfN>
boost::optional<Elf_Sym<ElfN>>
ElfGnuHashTable<ElfN>::find_symbol(const ElfSymbolTable<ElfN> & sym_table,
                                   const ElfStringTable<ElfN> & str_table,
                                   const std::string & sym_name) const
{
	const auto wordsize = sizeof(Elf_Word<ElfN>);
	const auto sym_hash = elf_gnu_hash(sym_name.c_str());
	const auto bucket_index = sym_hash % table_.nbuckets;
	const auto bucket_offs = buckets_rva_ + bucket_index * wordsize;
	auto chain_iter = image_->template load_from<Elf_Word<ElfN>>(bucket_offs);
	if (chain_iter == 0) return boost::none;
	Elf_Word<ElfN> chain_hash;
	do {
		const auto chain_index = chain_iter - table_.symndx;
		const auto chain_offs = chains_rva_ + chain_index * wordsize;
		chain_hash = image_->template load_from<Elf_Word<ElfN>>(chain_offs);
		if (((chain_hash ^ sym_hash) & ~1) == 0) {
			const auto sym = sym_table.get_symbol(chain_iter);
			if (str_table.get_string(sym.st_name) == sym_name)
				return sym;
		}
		++chain_iter;
	} while ((chain_hash & 1) == 0);
	return boost::none;
}

template <class ElfN, typename T>
ElfObjectRange<ElfN, T>::ElfObjectRange()
	: image_ { nullptr }, entsize_ { 0 }, begin_ { 0 }, end_ { 0 } {}

template <class ElfN, typename T>
ElfObjectRange<ElfN, T>::ElfObjectRange(const ElfImageR<ElfN> & image,
                                        std::uintptr_t base, std::size_t entsize,
                                        unsigned int count)
	: image_ { &image }, entsize_ { entsize }
	, begin_ { base }, end_ { base + entsize * count } {}

template <class ElfN, typename T>
auto ElfObjectRange<ElfN, T>::begin() const -> iterator
{
	return { *this, begin_ };
}

template <class ElfN, typename T>
auto ElfObjectRange<ElfN, T>::end() const -> iterator
{
	return { *this, end_ };
}

template <class ElfN, typename T>
ElfObjectIterator<ElfN, T>::ElfObjectIterator()
	: range_ { nullptr } {}

template <class ElfN, typename T>
ElfObjectIterator<ElfN, T>::ElfObjectIterator(ElfObjectRange<ElfN, T> const & range,
                                              std::uintptr_t reladdr)
	: range_ { &range }, reladdr_ { reladdr }
{
	if (reladdr < range.end_)
		range_->value_ = range_->image_->template load_from<T>(reladdr);
}

template <class ElfN, typename T>
void ElfObjectIterator<ElfN, T>::increment()
{
	reladdr_ += range_->entsize_;
	if (reladdr_ < range_->end_)
		range_->value_ = range_->image_->template load_from<T>(reladdr_);
}

template <class ElfN, typename T>
const T & ElfObjectIterator<ElfN, T>::dereference() const
{
	return range_->value_;
}

template <class ElfN, typename T>
bool ElfObjectIterator<ElfN, T>::equal(const ElfObjectIterator & other) const
{
	return range_ == other.range_ && reladdr_ == other.reladdr_;
}

inline auto ELF_ST_TYPE(const Elf32_Sym & sym)
{
	return ELF32_ST_TYPE(sym.st_info);
}

inline auto ELF_ST_TYPE(const Elf64_Sym & sym)
{
	return ELF64_ST_TYPE(sym.st_info);
}

inline auto ELF_ST_BIND(const Elf32_Sym & sym)
{
	return ELF32_ST_BIND(sym.st_info);
}

inline auto ELF_ST_BIND(const Elf64_Sym & sym)
{
	return ELF64_ST_BIND(sym.st_info);
}

inline auto ELF_ST_VISIBILITY(const Elf32_Sym & sym)
{
	return ELF32_ST_VISIBILITY(sym.st_other);
}

inline auto ELF_ST_VISIBILITY(const Elf64_Sym & sym)
{
	return ELF64_ST_VISIBILITY(sym.st_other);
}

inline auto ELF_R_TYPE(const Elf32_Rel & rel)
{
	return ELF32_R_TYPE(rel.r_info);
}

inline auto ELF_R_TYPE(const Elf32_Rela & rela)
{
	return ELF32_R_TYPE(rela.r_info);
}

inline auto ELF_R_TYPE(const Elf64_Rel & rel)
{
	return ELF64_R_TYPE(rel.r_info);
}

inline auto ELF_R_TYPE(const Elf64_Rela & rela)
{
	return ELF64_R_TYPE(rela.r_info);
}

inline auto ELF_R_SYM(const Elf32_Rel & rel)
{
	return ELF32_R_SYM(rel.r_info);
}

inline auto ELF_R_SYM(const Elf32_Rela & rela)
{
	return ELF32_R_SYM(rela.r_info);
}

inline auto ELF_R_SYM(const Elf64_Rel & rel)
{
	return ELF64_R_SYM(rel.r_info);
}

inline auto ELF_R_SYM(const Elf64_Rela & rela)
{
	return ELF64_R_SYM(rela.r_info);
}

}

#endif
