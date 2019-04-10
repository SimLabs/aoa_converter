/* 
    * Copyright (C) 2014 SimLabs LLC - All rights reserved. 
    * Unauthorized copying of this file or its part is strictly prohibited. 
    * For any information on this file or its part please contact: support@sim-labs.com
*/

#pragma once
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/filesystem.hpp>

namespace sys_utils
{

namespace fs   = boost::filesystem;
namespace bipc = boost::interprocess;
namespace bsys = boost::system;
namespace pt   = boost::posix_time;

template<class try_lockable>
struct scoped_try_lock
{
    explicit scoped_try_lock(try_lockable& lockable)
        : lockable_ (lockable)
        , owns_     (lockable.try_lock())
    {
    }

    ~scoped_try_lock()
    {
        if (owns_)
            lockable_.unlock();
    }

    bool owns() const
    {
        return owns_;
    }

private:
    try_lockable& lockable_;
    bool          owns_    ;
};

inline fs::path temp_folder()
{
    return fs::temp_directory_path();
}

inline fs::path home_folder()
{
        static fs::path home = 
#ifdef _WIN32
            fs::path(getenv("HOMEDRIVE")) / getenv("HOMEPATH");
#elif defined __linux__
            getenv("HOME");
#else
#error "unsupported platform"
#endif

    return home;
}

inline const char* simlabs_folder() 
{
    return ".simlabs.aux";
}

struct temp_file
{
    explicit temp_file(std::string base_name, bool home = true)
        : path_(base_path(home) / simlabs_folder() / base_name)
    {
        create_directories(path_.parent_path());
        create_file();
    }

    temp_file(temp_file&& other)
        : path_(std::move(other.path_))
    {
    }

    temp_file& operator=(temp_file&& other)
    {
        temp_file tmp(std::forward<temp_file>(other));
        swap(tmp);
        return *this;
    }

    void swap(temp_file& other)
    {
        fs::swap(path_, other.path_);
    }

    fs::path const& path() const 
    { 
        return path_; 
    }

    bool already_exists() const 
    { 
        return file_exists(path_);
    }

private:
    fs::path base_path(bool home)
    {
        return home ? home_folder() : temp_folder();
    }

    void create_file()
    {
        if (!file_exists(path_))
        {
            if (FILE* file = fopen(path_.string().c_str(), "a+"))
                fclose(file);
        }
    }

    bool file_exists(fs::path const& p) const
    {
        bsys::error_code ec;
        return fs::exists(p, ec);
    }

private:
    fs::path path_;
};

struct named_lock
    : boost::noncopyable
{
    explicit named_lock(string name)
        : file_ (name)
        , flock_(file_.path().string().c_str())
    {
    }

    named_lock(named_lock&& other)
        : file_ (std::move(other.file_ )) 
        , flock_(std::move(other.flock_))
    {
    }

    named_lock& operator=(named_lock&& other)
    {
        named_lock nl(std::forward<named_lock>(other));
        swap(nl);
        return *this;
    }

    // public member functions
    void swap(named_lock& other)
    {
        file_.swap(other.file_);
        boost::swap(flock_, other.flock_);
    }

    fs::path const& path() const 
    {
        return file_.path();
    }

    void lock       ()                                          { flock_.lock(); }
    bool try_lock   ()                                          { return flock_.try_lock(); }
    void unlock     ()                                          { flock_.unlock(); }

    void lock_sharable      ()                                          { flock_.lock_sharable(); }
    bool try_lock_sharable  ()                                          { return flock_.try_lock_sharable(); }
    void unlock_sharable    ()                                          { flock_.unlock_sharable(); }

    bool timed_lock (const boost::posix_time::ptime& abs_time)  
    { 
        try { return flock_.timed_lock(abs_time); }
        catch(boost::interprocess::interprocess_exception const& /*e*/){ return false; }
    }

    bool timed_lock_sharable(const boost::posix_time::ptime& abs_time)  
    { 
        try { return flock_.timed_lock_sharable(abs_time); }
        catch(boost::interprocess::interprocess_exception const& /*e*/){ return false; }
    }

private:
    temp_file       file_;
    bipc::file_lock flock_;
};

struct process_singleton
{
    explicit process_singleton(string name)
        : flock_(name)
        , time_ (update_time(flock_.path(), name))
        , lock_ (flock_)
    {
        VerifyMsg(lock_.owns(), "Process singleton cannot acquire lock \"" << name << "\"");
    }

private:
    typedef scoped_try_lock<named_lock> scoped_try_lock_t;

private:
    pt::ptime update_time(fs::path path, string name)
    {
        // owner of the singleton
        // yes, lock only to check
        // flushing to file will lead to err if file is locked (https://svn.boost.org/trac/boost/ticket/2796)
        VerifyMsg(scoped_try_lock_t(flock_).owns(), "Process singleton cannot acquire lock \"" << name << "\""); 
        std::ofstream(path.string()) << pt::microsec_clock::universal_time();

        return pt::from_time_t(last_write_time(path));
    }

private:
    named_lock          flock_;
    pt::ptime           time_;
    scoped_try_lock_t   lock_;
};

struct process_singleton_check
{
    explicit process_singleton_check(string name)
        : flock_(name)
    {
    }

    process_singleton_check(process_singleton_check&& other)
        : flock_(std::move(other.flock_))
    {
    }

    boost::optional<pt::ptime> check()
    {
        scoped_try_lock<named_lock> lock(flock_);
        if (lock.owns())
            return boost::none;

        return pt::from_time_t(last_write_time(flock_.path()));
    }

private:
    named_lock flock_;
};


} // sys_utils
