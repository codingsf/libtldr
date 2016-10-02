#ifndef TLDR_SRC_ELF_HASH_HPP_
#define TLDR_SRC_ELF_HASH_HPP_

#include <elf.h>

namespace tldr {

struct Elf32_Hash
{
	Elf32_Word nbuckets;
	Elf32_Word nchains;
};

struct Elf64_Hash
{
	Elf64_Word nbuckets;
	Elf64_Word nchains;
};

struct Elf32_GnuHash
{
	Elf32_Word nbuckets;
	Elf32_Word symndx;
	Elf32_Word maskwords;
	Elf32_Word gnu_shift;
};

struct Elf64_GnuHash
{
	Elf64_Word nbuckets;
	Elf64_Word symndx;
	Elf64_Word maskwords;
	Elf64_Word gnu_shift;
};

unsigned long elf_hash(const char * name);
unsigned long elf_gnu_hash(const char * name);

}

#endif
