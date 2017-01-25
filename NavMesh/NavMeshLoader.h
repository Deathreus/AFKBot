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
		CFileHandle(FILE *fp) : fp_(fp) {}
		CFileHandle(FileHandle_t fh) : fh_(fh) {}
		~CFileHandle()
		{
			if (fp_)
			{
				fclose(fp_);
				fp_ = NULL;
			}
			if (fh_)
			{
				filesystem->Close(fh_);
				fh_ = NULL;
			}
		}

		bool FileError()
		{
			return (fp_ == NULL || ferror(fp_));
		}

		bool HandleError()
		{
			return (fh_ == NULL || !filesystem->IsOk(fh_));
		}

		size_t Read(void *output, int elementSize, int elementCount)
		{
			if (!FileError())
			{
				return fread(output, elementSize, elementCount, fp_);
			}

			if (!HandleError())
			{
				return (size_t)filesystem->Read(output, elementSize, fh_);
			}

			smutils->LogError(myself, "Fatal error attempting to read file data!");
			return 0;
		}

	private:
		FILE *fp_;
		FileHandle_t fh_;
	};
};

#endif