#ifndef __war3source_navmeshloader_h__
#define __war3source_navmeshloader_h__

#include "public/INavMesh.h"

#include "../extension.h"


extern IFileSystem *filesystem;

class CNavMeshLoader
{
public:
	static INavMesh *Load(char *, int);
	static bool Save(INavMesh *);

private:

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

		size_t Read(void *output, int elementSize)
		{
			if (!HandleError())
			{
				return filesystem->Read(output, elementSize, this->fh);
			}

			if (!FileError())
			{
				return fread(output, elementSize, 1, this->fp);
			}

			smutils->LogError(myself, "Fatal error attempting to read file data!");
			return 0;
		}

		void Write(void *input, int elementSize)
		{
			if (!HandleError())
			{
				filesystem->Write(input, elementSize, this->fh);
			}
			else if (!FileError())
			{
				fwrite(input, elementSize, 1, this->fp);
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

#endif