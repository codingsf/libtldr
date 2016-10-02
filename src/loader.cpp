#include <config.h>
#include <tldr/loader.hpp>

#include <tldr/module.hpp>

namespace tldr {

class NullModuleResolver final : public ModuleResolver
{
public:
	virtual std::shared_ptr<Module> get_module(const std::string & name) const override;
};

std::shared_ptr<Module> NullModuleResolver::get_module(const std::string & name) const
{
	return nullptr;
}

namespace {
	const NullModuleResolver null_module_resolver {};
}

ModuleResolver::~ModuleResolver() = default;

Loader::Loader() : resolver_ { &null_module_resolver } {}

std::shared_ptr<Module> Loader::get_module(const std::string & name) const
{
	const auto mod_iter = modules_.find(name);
	if (mod_iter != modules_.end()) {
		const auto mod_ref = mod_iter->second.lock();
		if (mod_ref) return mod_ref;
		modules_.erase(mod_iter);
	}
	return resolver_->get_module(name);
}

void Loader::set_module(const std::string & name, std::shared_ptr<Module> module)
{
	const auto result = modules_.emplace(name, std::move(module));
	if (!result.second)
		result.first->second = std::move(module);
}

void Loader::set_module_resolver(const ModuleResolver * resolver)
{
	resolver_ = resolver ? resolver : &null_module_resolver;
}

void Loader::remove_module(const std::string & name)
{
	modules_.erase(name);
}

}
