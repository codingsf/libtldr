#ifndef TLDR_LOADER_HPP_
#define TLDR_LOADER_HPP_

#include <tldr/export.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace tldr {

class Module;

class TLDR_EXPORT ModuleResolver
{
public:
	virtual ~ModuleResolver();
	virtual std::shared_ptr<Module> get_module(const std::string & name) const = 0;
};

class TLDR_EXPORT Loader : public ModuleResolver
{
public:
	Loader();

	virtual std::shared_ptr<Module> get_module(const std::string & name) const override;

	void set_module(const std::string & name, std::shared_ptr<Module> module);
	void remove_module(const std::string & name);

	void set_module_resolver(const ModuleResolver * resolver);

private:
	mutable std::unordered_map<std::string, std::weak_ptr<Module>> modules_;
	const ModuleResolver * resolver_;
};

}

#endif
