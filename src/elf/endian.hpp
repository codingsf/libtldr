#ifndef TLDR_SRC_ELF_ENDIAN_HPP_
#define TLDR_SRC_ELF_ENDIAN_HPP_

#include "../endian.hpp"
#include "hash.hpp"

#include <elf.h>

#include <cstddef>
#include <type_traits>

namespace tldr {

template <class ElfN_Ehdr>
struct impl_symmetric_serializable_for<ElfN_Ehdr,
	std::enable_if_t<
		std::is_same<ElfN_Ehdr, Elf32_Ehdr>::value
		|| std::is_same<ElfN_Ehdr, Elf64_Ehdr>::value
>>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && ehdr, io_ptr mem, std::size_t size)
	{
		io::ref(ehdr.e_ident,     offsetof(ElfN_Ehdr, e_ident    )).transfer(mem, size);
		io::ref(ehdr.e_type,      offsetof(ElfN_Ehdr, e_type     )).transfer(mem, size);
		io::ref(ehdr.e_machine,   offsetof(ElfN_Ehdr, e_machine  )).transfer(mem, size);
		io::ref(ehdr.e_version,   offsetof(ElfN_Ehdr, e_version  )).transfer(mem, size);
		io::ref(ehdr.e_entry,     offsetof(ElfN_Ehdr, e_entry    )).transfer(mem, size);
		io::ref(ehdr.e_phoff,     offsetof(ElfN_Ehdr, e_phoff    )).transfer(mem, size);
		io::ref(ehdr.e_shoff,     offsetof(ElfN_Ehdr, e_shoff    )).transfer(mem, size);
		io::ref(ehdr.e_flags,     offsetof(ElfN_Ehdr, e_flags    )).transfer(mem, size);
		io::ref(ehdr.e_ehsize,    offsetof(ElfN_Ehdr, e_ehsize   )).transfer(mem, size);
		io::ref(ehdr.e_phentsize, offsetof(ElfN_Ehdr, e_phentsize)).transfer(mem, size);
		io::ref(ehdr.e_phnum,     offsetof(ElfN_Ehdr, e_phnum    )).transfer(mem, size);
		io::ref(ehdr.e_shentsize, offsetof(ElfN_Ehdr, e_shentsize)).transfer(mem, size);
		io::ref(ehdr.e_shnum,     offsetof(ElfN_Ehdr, e_shnum    )).transfer(mem, size);
		io::ref(ehdr.e_shstrndx,  offsetof(ElfN_Ehdr, e_shstrndx )).transfer(mem, size);
		return mem + sizeof(ElfN_Ehdr);
	}
};

template <class ElfN_Phdr>
struct impl_symmetric_serializable_for<ElfN_Phdr,
	std::enable_if_t<
		std::is_same<ElfN_Phdr, Elf32_Phdr>::value
		|| std::is_same<ElfN_Phdr, Elf64_Phdr>::value
>>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && phdr, io_ptr mem, std::size_t size)
	{
		io::ref(phdr.p_type,   offsetof(ElfN_Phdr, p_type  )).transfer(mem, size);
		io::ref(phdr.p_offset, offsetof(ElfN_Phdr, p_offset)).transfer(mem, size);
		io::ref(phdr.p_vaddr,  offsetof(ElfN_Phdr, p_vaddr )).transfer(mem, size);
		io::ref(phdr.p_paddr,  offsetof(ElfN_Phdr, p_paddr )).transfer(mem, size);
		io::ref(phdr.p_filesz, offsetof(ElfN_Phdr, p_filesz)).transfer(mem, size);
		io::ref(phdr.p_memsz,  offsetof(ElfN_Phdr, p_memsz )).transfer(mem, size);
		io::ref(phdr.p_flags,  offsetof(ElfN_Phdr, p_flags )).transfer(mem, size);
		io::ref(phdr.p_align,  offsetof(ElfN_Phdr, p_align )).transfer(mem, size);
		return mem + sizeof(ElfN_Phdr);
	}
};

template <class ElfN_Dyn>
struct impl_symmetric_serializable_for<ElfN_Dyn,
	std::enable_if_t<
		std::is_same<ElfN_Dyn, Elf32_Dyn>::value
		|| std::is_same<ElfN_Dyn, Elf64_Dyn>::value
>>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && dyn, io_ptr mem, std::size_t size)
	{
		io::ref(dyn.d_tag,      offsetof(ElfN_Dyn, d_tag)).transfer(mem, size);
		io::ref(dyn.d_un.d_val, offsetof(ElfN_Dyn, d_un )).transfer(mem, size);
		io::ref(dyn.d_un.d_ptr, offsetof(ElfN_Dyn, d_un )).transfer(mem, size);
		return mem + sizeof(ElfN_Dyn);
	}
};

template <class ElfN_Sym>
struct impl_symmetric_serializable_for<ElfN_Sym,
	std::enable_if_t<
		std::is_same<ElfN_Sym, Elf32_Sym>::value
		|| std::is_same<ElfN_Sym, Elf64_Sym>::value
>>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && sym, io_ptr mem, std::size_t size)
	{
		io::ref(sym.st_name,  offsetof(ElfN_Sym, st_name )).transfer(mem, size);
		io::ref(sym.st_value, offsetof(ElfN_Sym, st_value)).transfer(mem, size);
		io::ref(sym.st_size,  offsetof(ElfN_Sym, st_size )).transfer(mem, size);
		io::ref(sym.st_info,  offsetof(ElfN_Sym, st_info )).transfer(mem, size);
		io::ref(sym.st_other, offsetof(ElfN_Sym, st_other)).transfer(mem, size);
		io::ref(sym.st_shndx, offsetof(ElfN_Sym, st_shndx)).transfer(mem, size);
		return mem + sizeof(ElfN_Sym);
	}
};

template <class ElfN_Rel>
struct impl_symmetric_serializable_for<ElfN_Rel,
	std::enable_if_t<
		std::is_same<ElfN_Rel, Elf32_Rel>::value
		|| std::is_same<ElfN_Rel, Elf64_Rel>::value
>>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && rel, io_ptr mem, std::size_t size)
	{
		io::ref(rel.r_offset, offsetof(ElfN_Rel, r_offset)).transfer(mem, size);
		io::ref(rel.r_info,   offsetof(ElfN_Rel, r_info  )).transfer(mem, size);
		return mem + sizeof(ElfN_Rel);
	}
};

template <class ElfN_Rela>
struct impl_symmetric_serializable_for<ElfN_Rela,
	std::enable_if_t<
		std::is_same<ElfN_Rela, Elf32_Rela>::value
		|| std::is_same<ElfN_Rela, Elf64_Rela>::value
>>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && rela, io_ptr mem, std::size_t size)
	{
		io::ref(rela.r_offset, offsetof(ElfN_Rela, r_offset)).transfer(mem, size);
		io::ref(rela.r_info,   offsetof(ElfN_Rela, r_info  )).transfer(mem, size);
		io::ref(rela.r_addend, offsetof(ElfN_Rela, r_addend)).transfer(mem, size);
		return mem + sizeof(ElfN_Rela);
	}
};

template <class ElfN_Hash>
struct impl_symmetric_serializable_for<ElfN_Hash,
	std::enable_if_t<
		std::is_same<ElfN_Hash, Elf32_Hash>::value
		|| std::is_same<ElfN_Hash, Elf64_Hash>::value
>>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && hash, io_ptr mem, std::size_t size)
	{
		io::ref(hash.nbuckets, offsetof(ElfN_Hash, nbuckets)).transfer(mem, size);
		io::ref(hash.nchains,  offsetof(ElfN_Hash, nchains )).transfer(mem, size);
		return mem + sizeof(ElfN_Hash);
	}
};

template <class ElfN_GnuHash>
struct impl_symmetric_serializable_for<ElfN_GnuHash,
	std::enable_if_t<
		std::is_same<ElfN_GnuHash, Elf32_GnuHash>::value
		|| std::is_same<ElfN_GnuHash, Elf64_GnuHash>::value
>>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && gnu_hash, io_ptr mem, std::size_t size)
	{
		io::ref(gnu_hash.nbuckets,  offsetof(ElfN_GnuHash, nbuckets )).transfer(mem, size);
		io::ref(gnu_hash.symndx,    offsetof(ElfN_GnuHash, symndx   )).transfer(mem, size);
		io::ref(gnu_hash.maskwords, offsetof(ElfN_GnuHash, maskwords)).transfer(mem, size);
		io::ref(gnu_hash.gnu_shift, offsetof(ElfN_GnuHash, gnu_shift)).transfer(mem, size);
		return mem + sizeof(ElfN_GnuHash);
	}
};

}

#endif
