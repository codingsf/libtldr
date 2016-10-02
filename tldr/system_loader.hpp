#ifndef TLDR_SYSTEMLOADER_HPP_
#define TLDR_SYSTEMLOADER_HPP_

#include <tldr/loader.hpp>

namespace tldr {

class TLDR_EXPORT SystemLoader final : public ModuleResolver
{
public:
	virtual std::shared_ptr<Module> get_module(const std::string & name) const override;
};

TLDR_EXPORT extern const SystemLoader system_loader;

}

#endif
