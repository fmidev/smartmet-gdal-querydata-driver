#include "MappedFile.h"
#include "Exception.h"
#include <iostream>

#ifndef _MSC_VER
#include <sys/mman.h>
#include <unistd.h>
#endif

Fmi::MappedFile::MappedFile(const MappedFile::Params& params)
    : boost::iostreams::mapped_file(params)
    , m_path(params.path)
{
    madvise_nodump(params.offset);
}

Fmi::MappedFile::MappedFile(
    const std::string& path,
    std::ios_base::openmode mode,
    size_type length,
    boost::intmax_t offset)

    try

    : boost::iostreams::mapped_file(path, mode, length, offset)
    , m_path(path)
{
    madvise_nodump(offset);
}
catch (...)
{
    auto exception = Fmi::Exception::Trace(BCP, "Failed to map file");
    exception.addParameter("path", path);
    throw exception;
}

Fmi::MappedFile::MappedFile(
    const std::string& path,
    mapmode mode,
    size_type length,
    boost::intmax_t offset)

    try

      : boost::iostreams::mapped_file(path, mode, length, offset)
      , m_path(path)
{
    madvise_nodump(offset);
}
catch (...)
{
    auto exception = Fmi::Exception::Trace(BCP, "Failed to map file");
    exception.addParameter("path", path);
    exception.addParameter("mode", std::to_string(mode));
    exception.addParameter("length", std::to_string(length));
    exception.addParameter("offset", std::to_string(offset));
    throw exception;
}

Fmi::MappedFile::~MappedFile() = default;

void
Fmi::MappedFile::open(Params params)  try
{
    m_path = params.path;
    boost::iostreams::mapped_file::open(params);
    madvise_nodump(params.offset);
}
catch (...)
{
    auto exception = Fmi::Exception::Trace(BCP, "Failed to open mapped file");
    exception.addParameter("path", params.path);
    throw exception;
}

void
Fmi::MappedFile::open(
    const std::string& path,
    std::ios_base::openmode mode,
    size_type length,
    boost::intmax_t offset ) try
{
    m_path = path;
    boost::iostreams::mapped_file::open(path, mode, length, offset);
    madvise_nodump(offset);
}
catch (...)
{
    auto exception = Fmi::Exception::Trace(BCP, "Failed to open mapped file");
    exception.addParameter("path", path);
    exception.addParameter("mode", std::to_string(mode));
    exception.addParameter("length", std::to_string(length));
    exception.addParameter("offset", std::to_string(offset));
    throw exception;
}

void
Fmi::MappedFile::open(
    const std::string& path,
    mapmode mode,
    size_type length,
    boost::intmax_t offset) try
{
    m_path = path;
    boost::iostreams::mapped_file::open(path, mode, length, offset);
    madvise_nodump(offset);
}
catch (...)
{
    auto exception = Fmi::Exception::Trace(BCP, "Failed to open mapped file");
    exception.addParameter("path", path);
    exception.addParameter("mode", std::to_string(mode));
    exception.addParameter("length", std::to_string(length));
    exception.addParameter("offset", std::to_string(offset));
    throw exception;
}

void
Fmi::MappedFile::close() try
{
    boost::iostreams::mapped_file::close();
}
catch (...)
{
    auto exception = Fmi::Exception::Trace(BCP, "Failed to close mapped file");
    exception.addParameter("Path", m_path);
    throw exception;
}

void
Fmi::MappedFile::madvise_nodump(boost::intmax_t offset)
{
    if (boost::iostreams::mapped_file::is_open())
    {
        invoke_madvise(MADV_DONTDUMP, offset);
    }
}

void
Fmi::MappedFile::invoke_madvise(int adv, boost::intmax_t offset)
{
#ifndef _MSC_VER
    char *data_ptr = boost::iostreams::mapped_file::data()
        ? boost::iostreams::mapped_file::data()
        : const_cast<char*>(boost::iostreams::mapped_file::const_data());

    // madvise requires page-aligned address and size
    // When offset is non-zero, data() points to offset within the mapping
    const long page_size = sysconf(_SC_PAGESIZE);
    const boost::intmax_t offset_in_page = offset % page_size;
    
    // Calculate page-aligned base address
    char *aligned_address = data_ptr - offset_in_page;
    
    // Calculate actual mapped size (includes offset alignment padding)
    size_type aligned_size = size() + offset_in_page;
    
    int ret_val = madvise(aligned_address, aligned_size, adv);
    if (ret_val < 0)
    {
        const int err = errno;
        char tmp[80];
        Fmi::Exception exception(BCP, "madvise() failed");
        exception.addParameter("path", m_path);
        exception.addParameter("errno", std::to_string(err));
        exception.addParameter("description", strerror_r(err, tmp, sizeof tmp));
        snprintf(tmp, sizeof(tmp), "%p", (void*)aligned_address);
        exception.addParameter("address", tmp);
        exception.addParameter("size", std::to_string(aligned_size));
        exception.addParameter("offset", std::to_string(offset));
        exception.addParameter("page_size", std::to_string(page_size));
        std::cerr << exception << std::endl;
    }
#else
    (void)adv;
    (void)offset;
#endif
}

