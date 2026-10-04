#include "rvpch.h"
#include "ScriptCore.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

#include "ScriptConnector.h"
#include "MyRevoke/Utility/ExecutablePath.h"

namespace Revoke
{
	namespace ScriptCore
	{

		struct ScriptCoreData
		{
			MonoDomain* RootDomain = nullptr;
			MonoDomain* AppDomain = nullptr;

			MonoAssembly* CoreAssembly = nullptr;
			MonoImage* CoreAssemblyImage = nullptr;

			MonoClass* EntityMonoClass = nullptr;
		};

		static ScriptCoreData s_Data;

		static char* ReadBytes(const std::string& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				RV_CORE_ASSERT(false, "Failed to open the file");
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = end - stream.tellg();

			if (size == 0)
			{
				RV_CORE_ASSERT(false, "File is empty");
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = size;
			return buffer;
		}

		static MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
			mono_image_close(image);

			// Don't forget to free the file data
			delete[] fileData;

			return assembly;
		}

		static void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				printf("%s.%s\n", nameSpace, name);
			}
		}
		
		static MonoObject* InstantiateClass(MonoClass* monoClass) //TODO: maybe needed to be used globaly so move to .h file
		{

			MonoObject* monoObject = mono_object_new(s_Data.AppDomain, monoClass);
			mono_runtime_object_init(monoObject);
			return monoObject;

		}

		void Init()
		{
			std::string assembliesPath = (GetExecutableDirectory() / "mono" / "lib").string();
			mono_set_assemblies_path(assembliesPath.c_str());
			
			MonoDomain* rootDomain = mono_jit_init("RevokeRuntime");
			if (rootDomain == nullptr)
			{
				RV_CORE_ASSERT(false, "Failed to initialize Root Domain");
				return;
			}

			// Store the root domain pointer
			s_Data.RootDomain = rootDomain;

			LoadAssembly((GetExecutableDirectory() / "resourses" / "scripts" / "MyRevoke-ScriptCore.dll").string());


			//Example
			ScriptConnector::ConnectFunctions(); // Connects cpp function defined in script connector to c# functions

			s_Data.EntityMonoClass = mono_class_from_name(s_Data.CoreAssemblyImage, "Revoke", "Entity"); // Gets class

			MonoObject* monoObject = InstantiateClass(s_Data.EntityMonoClass); // Calls constructor

			MonoMethod* PrintMessegeFunc = mono_class_get_method_from_name(s_Data.EntityMonoClass, "PrintMessege", 0); // Gets function
			mono_runtime_invoke(PrintMessegeFunc, monoObject, nullptr, nullptr); // Calls function

			MonoMethod* PrintCustomMessegeFunc = mono_class_get_method_from_name(s_Data.EntityMonoClass, "PrintCustomMessege", 1); // Gets function
			int value = 5;
			void* params[1]
			{
				&value
			};
			mono_runtime_invoke(PrintCustomMessegeFunc, monoObject, params, nullptr); // Calls function

		}

		void LoadAssembly(const std::string& path)
		{
			char myAppDomain[] = "RevokeScriptRuntime";
			s_Data.AppDomain = mono_domain_create_appdomain(myAppDomain, nullptr);
			mono_domain_set(s_Data.AppDomain, true);

			s_Data.CoreAssembly = LoadCSharpAssembly(path);
			//PrintAssemblyTypes(s_Data.CoreAssembly);

			s_Data.CoreAssemblyImage = mono_assembly_get_image(s_Data.CoreAssembly); 
		}

		void Shutdown()
		{
		
		}

	};
}
