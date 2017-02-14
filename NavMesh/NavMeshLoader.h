#ifndef __war3source_navmeshloader_h__
#define __war3source_navmeshloader_h__

#include "public\INavMeshLoader.h"
#include "public\INavMesh.h"
#include <iostream>

#include "..\extension.h"


class CNavMeshLoader : public INavMeshLoader 
{
public:
	CNavMeshLoader(const char *mapName);
	~CNavMeshLoader();

	INavMesh *Load(char *error, int errorMaxlen);

private:
	char mapName[100];
	unsigned int bytesRead;

	class CFileHandle
	{
	public:
		CFileHandle(FILE *fp) : fp(fp) {}
		CFileHandle(FileHandle_t fh) : fh(fh) {}
		~CFileHandle()
		{
			if (fp)
			{
				fclose(fp);
				fp = NULL;
			}
			if (fh)
			{
				filesystem->Close(fh);
				fh = NULL;
			}
		}

		bool FileError()
		{
			return (fp == NULL || ferror(fp));
		}

		bool HandleError()
		{
			return (fh == NULL || !filesystem->IsOk(fh));
		}

		size_t Read(void *output, int elementSize, int elementCount)
		{
			if (!FileError())
			{
				return fread(output, elementSize, elementCount, fp);
			}

			if (!HandleError())
			{
				return (size_t)filesystem->Read(output, elementSize, fh);
			}

			smutils->LogError(myself, "Fatal error attempting to read file data!");
			return 0;
		}

	protected:
		FILE *fp;
		FileHandle_t fh;
	};
};

#endif