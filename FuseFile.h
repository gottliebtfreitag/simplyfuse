#pragma once

#include <errno.h>
#include <iterator>
#include <unistd.h>

#include <mutex>
#include <functional>

namespace simplyfuse {

struct FuseFS;

// subclass this struct to implement functionality represented by a file
struct FuseFile {
	FuseFile() = default;
	FuseFile(FuseFile const&) = delete;
	FuseFile(FuseFile &&) = delete;
	FuseFile operator=(FuseFile const&) = delete;
	FuseFile operator=(FuseFile&&) = delete;

	virtual ~FuseFile();
	virtual int onOpen();
	virtual int onClose();
	virtual int onRead(char* buf, std::size_t size, off_t offset);
	virtual int onWrite(const char* buf, std::size_t size, off_t offset);
	virtual std::size_t getSize();
	virtual int onTruncate(off_t offset);

	virtual int getFilePermissions();

	friend class FuseFS;
protected:
	FuseFS* fuseFS {nullptr};
};

struct SimpleROFile : FuseFile {
	using FuseFile::FuseFile;
	using ReadCallback = std::function<std::string(void)>;

	SimpleROFile(std::string const& _content="", ReadCallback readCallback={}) : content{_content}, readCB{readCallback} {};
	virtual ~SimpleROFile() = default;
	int onRead(char* buf, std::size_t size, off_t offset) override;
	int getFilePermissions() override;

	void setContent(std::string const& content);
	std::string const& getContent() const;

	std::unique_lock<std::mutex> lock() const {
		return std::unique_lock{mutex};
	}
protected:
	mutable std::mutex mutex;
	std::string content;
	ReadCallback readCB{};
};

struct SimpleWOFile : FuseFile {
	using FuseFile::FuseFile;
	virtual ~SimpleWOFile() = default;
	int onTruncate(off_t offset);
	int getFilePermissions() override;
};

struct SimpleRWFile : SimpleROFile {
	using SimpleROFile::SimpleROFile;
	virtual ~SimpleRWFile() = default;
	int onWrite(const char* buf, std::size_t size, off_t offset);
	int onTruncate(off_t offset);
	int getFilePermissions() override;
};

} /* namespace simplyfile */
