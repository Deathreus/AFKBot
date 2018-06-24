#pragma once

#include "public/INavMesh.h"

#include "../extension.h"
#include "utlbuffer.h"


extern IFileSystem *filesystem;

class CNavMeshLoader
{
public:
	static INavMesh *Load(char *, int);
	static bool Save(INavMesh *);

private:
	static unsigned int bytesRead;

	// Dynamically choose to use OS filesystem or Valves
	class CFileHandle
	{
	public:
		CFileHandle(FILE *_fp)
		{
			this->fp = _fp;
			this->fh = NULL;
		}
		CFileHandle(FileHandle_t _fh)
		{
			this->fp = NULL;
			this->fh = _fh;
		}
		~CFileHandle()
		{
			if (this->fp)
			{
				fclose(this->fp);
				this->fp = NULL;
			}
			if (this->fh)
			{
				filesystem->Close(this->fh);
				this->fh = NULL;
			}
		}

		bool FileError()
		{
			return (this->fp == NULL || ferror(this->fp));
		}

		bool HandleError()
		{
			return (this->fh == NULL || !filesystem->IsOk(this->fh));
		}

		size_t Read(void *output, int elementSize, int elementCount)
		{
			if (!HandleError())
			{
				CNavMeshLoader::bytesRead += elementSize;
				return (size_t)filesystem->Read(output, elementSize, this->fh);
			}

			if (!FileError())
			{
				CNavMeshLoader::bytesRead += elementSize;
				return fread(output, elementSize, elementCount, this->fp);
			}

			smutils->LogError(myself, "Fatal error attempting to read file data!");
			return 0;
		}

		void Write(void *input, int elementSize, int elementCount)
		{
			if (!HandleError())
			{
				filesystem->Write(input, elementSize, this->fh);
			}
			else if (!FileError())
			{
				fwrite(input, elementSize, elementCount, this->fp);
			}
			else
			{
				smutils->LogError(myself, "Fatal error attempting to write to file!");
			}
		}

	private:
		FILE *fp;
		FileHandle_t fh;
	};
};

inline FILE *MkDirIfNotExist(const char *szPath, const char *szMode)
{
	FILE *file = fopen(szPath, szMode);

	if (!file || ferror(file))
	{
	#ifndef __linux__
		char *delimiter = "\\";
	#else
		char *delimiter = "/";
	#endif

		char szFolderName[1024];
		int folderNameSize = 0;
		szFolderName[0] = 0;

		int iLen = strlen(szPath);

		int i = 0;

		while (i < iLen)
		{
			while ((i < iLen) && (szPath[i] != *delimiter))
			{
				szFolderName[folderNameSize++] = szPath[i];
				i++;
			}

			if (i == iLen)
				break;

			i++;
			szFolderName[folderNameSize++] = *delimiter;//next
			szFolderName[folderNameSize] = 0;

	#ifndef __linux__
			mkdir(szFolderName);
	#else
			mkdir(szFolderName, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	#endif   
		}

		file = fopen(szPath, szMode);

		if (!file || ferror(file))
		{
			smutils->LogError(myself, "Failed to create file at '%s' with mode %s", szPath, szMode);
			return NULL;
		}
	}

	return file;
}
