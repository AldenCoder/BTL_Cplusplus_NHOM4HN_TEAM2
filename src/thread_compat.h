#ifndef THREAD_COMPAT_H
#define THREAD_COMPAT_H

// For MinGW compatibility - use simple no-op mutex for single-threaded operation
namespace std {
    class mutex {
    public:
        mutex() = default;
        ~mutex() = default;
        void lock() { /* no-op for single-threaded compatibility */ }
        void unlock() { /* no-op for single-threaded compatibility */ }
        
        // Non-copyable
        mutex(const mutex&) = delete;
        mutex& operator=(const mutex&) = delete;
    };
    
    template<typename T>
    class lock_guard {
    private:
        T& mutex_;
    public:
        explicit lock_guard(T& m) : mutex_(m) {
            mutex_.lock();
        }
        
        ~lock_guard() {
            mutex_.unlock();
        }
        
        // Non-copyable
        lock_guard(const lock_guard&) = delete;
        lock_guard& operator=(const lock_guard&) = delete;
    };
}

#endif // THREAD_COMPAT_H
