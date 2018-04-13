#define PACKAGE_GPC_TOOL 1

#ifdef PACKAGE_GPC_TOOL
#include <stdio.h>
#include <fc\filesystem.hpp>
#include <fc\exception\exception.hpp>
#include <string>
#include <blockchain\Exceptions.hpp>
#include <iostream>
#include <fstream>
#include <uvm\uvm_api.h>
#include <fc\io\json.hpp>
#include <blockchain\ContractEntry.hpp>
#include <client\ClientImpl.hpp>
using namespace std;

struct CodeInfo {
	std::vector<std::string> api;
	std::vector<std::string> offline_api;
	std::vector<std::string> event;
	std::map<std::string, fc::unsigned_int> storage_properties_types;

	std::map<std::string, std::vector<fc::unsigned_int>> api_args_types;
};

FC_REFLECT(::CodeInfo,
(api)
(offline_api)
(event)
(storage_properties_types)
(api_args_types)
)

int main(int argc,char** argv)
{
	try{
	if (argc < 3)
	{
		printf("No enough params\n");
		return 0;
	}
	fc::path code_file(argv[1]);
	fc::path codeinfo_file(argv[2]);
	if (!fc::exists(code_file) || !fc::exists(codeinfo_file))
		printf( "the file not exist!\n");
	string out_filename = code_file.string();

	size_t pos;

	pos = code_file.string().find_last_of('.');
	if ((pos != string::npos))
	{
		out_filename = code_file.string().substr(0, pos) + ".gpc";
	}
	else
	{
		FC_THROW_EXCEPTION(uvm::blockchain::invalid_contract_filename, "contract source file name should end with .lua or .uvm");
	}
	std::ifstream file(code_file.string().c_str(), std::ifstream::binary);
	file.seekg(0, file.end);
	auto length = file.tellg();
	UvmModuleByteStream* p_new_stream = new UvmModuleByteStream();
	if (p_new_stream == NULL)
	{
		printf("Create UvmModuleByteStream Failed\n");
		return 0;
	}
		
	p_new_stream->buff.resize(length);
	file.seekg(0, file.beg);
	file.read(p_new_stream->buff.data(), length);
	auto info = fc::json::from_file(codeinfo_file).as<CodeInfo>();
	p_new_stream->contract_apis = info.api;
	p_new_stream->contract_emit_events = info.event;
	p_new_stream->offline_apis = info.offline_api;
	auto it = info.api_args_types.begin();
	while (it != info.api_args_types.end())
	{
		vector<UvmTypeInfoEnum> vec;
		for (auto type_it : it->second)
		{

			vec.push_back((UvmTypeInfoEnum)type_it);
		}
		p_new_stream->contract_api_arg_types.insert(std::make_pair(it->first, vec));
		it++;
	}
	p_new_stream->contract_storage_properties.clear();
	for (const auto & p : info.storage_properties_types)
	{
		p_new_stream->contract_storage_properties[p.first] = p.second;
	}
	if (uvm::client::detail::ClientImpl(nullptr, "").save_code_to_file(out_filename, p_new_stream, NULL) < 0)
	{
		delete p_new_stream;
		p_new_stream = nullptr;
		FC_THROW_EXCEPTION(uvm::blockchain::save_bytecode_to_gpcfile_fail, "");
	}
	if (p_new_stream)
		delete p_new_stream;
	printf("%s\n", out_filename.c_str()) ;
	return 0;
	}
	catch (std::exception &e)
	{
		printf(e.what());
		printf("Unknown Error\n");
	}
}
#endif