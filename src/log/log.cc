#include "log.h"

Log::Log() {
    line_count_ = 0;
    is_async_ = false;
    write_thread_ = nullptr;
    deque_ = nullptr;
    to_day_ = 0;
    fp_ = nullptr;
}

Log::~Log() {
    if(write_thread_ && write_thread_->joinable()) {
        while (!deque_->empty()) {
            deque_->flush();
        }
        deque_->Close();
        write_thread_->join();
    }
    if (fp_) {
        std::lock_guard<std::mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}

int Log::GetLevel() {
    std::lock_guard<std::mutex> locker(mtx_);
    return level_;
}

void Log::SetLevel(int level) {
    std::lock_guard<std::mutex> locker(mtx_);
    level_ = level;
}

void Log::init(int level, const char* path, 
                const char* suffix, int max_queue_size) {
    is_open_ = true;
    level_ = level;
    if (max_queue_size > 0) {
        is_async_ = true;
        if (!deque_) {
            std::unique_ptr<BlockDeque<std::string>> new_deque(new BlockDeque<std::string>);
            deque_ = std::move(new_deque);

            std::unique_ptr<std::thread> new_thread(new std::thread(FlushLogThread));
            write_thread_ = std::move(new_thread);
        }
    } else {
        is_async_ = false;
    }

    line_count_ = 0;

    time_t timer = time(nullptr);
    struct tm *sys_time = localtime(&timer);
    struct tm t = *sys_time;
    path_ = path;
    suffix_ = suffix;
    char file_name[LOG_NAME_LEN] = {0};
    snprintf(file_name, LOG_NAME_LEN-1, "%s%04d_%02d_%02d%s", path_, t.tm_year+1900, t.tm_mon+1, t.tm_mday, suffix_);
    to_day_ = t.tm_mday;
    {
        std::lock_guard<std::mutex> locker(mtx_);
        buff_.RetrieveAll();
        if (fp_) {
            flush();
            fclose(fp_);
        }

        fp_ = fopen(file_name, "a");
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(file_name, "a");
        }
        assert(fp_ != nullptr);
    }
}

void Log::write(int level, const char *format,...) {
    struct timeval now = {0};
    gettimeofday(&now, nullptr);
    time_t tsec = now.tv_sec;
    struct tm *sys_time = localtime(&tsec);
    struct tm t = *sys_time;
    va_list valist;

    if (to_day_ != t.tm_mday || (line_count_ % MAX_LINES == 0)) {
        std::unique_lock<std::mutex> locker(mtx_);
        locker.unlock();

        char new_file[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (to_day_ != t.tm_mday) {
            // 72 = 36 + 36: length of tail and suffix
            snprintf(new_file, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            to_day_ = t.tm_mday;
            line_count_ = 0;
        } else {
            snprintf(new_file, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (line_count_ / MAX_LINES), suffix_);
        }

        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(new_file, "a");
        assert(fp_ != nullptr);
    }

    {
        std::unique_lock<std::mutex> locker(mtx_);
        line_count_+=1;
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        
        buff_.HasWritten(n);
        AppendLogLevelTitle_(level);

        va_start(valist, format);
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, valist);
        va_end(valist);

        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if (is_async_ && deque_ && !deque_->full()) {
            deque_->push_back(buff_.RetrieveAllToStr());
        } else {
            fputs(buff_.Peek(), fp_);
        }
        buff_.RetrieveAll();
    }
}

Log* Log::Instance() {
    static Log inst;
    return &inst;
}

void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite_();
}

void Log::flush() {
    if (is_async_) {
        deque_->flush();
    }
    fflush(fp_);
}

bool Log::IsOpen() {
    return is_open_;
}

void Log::AppendLogLevelTitle_(int level) {
    switch (level) {
        case 0:
            buff_.Append("[debug]: ", 9);
            break;
        case 1:
            buff_.Append("[info] : ", 9);
            break;
        case 2:
            buff_.Append("[warn] : ", 9);
            break;
        case 3:
            buff_.Append("[error]: ", 9);
            break;
        default:
            buff_.Append("[info] : ", 9);
            break;
    }
}

void Log::AsyncWrite_() {
    std::string str = "";
    while (deque_->pop(str)) {
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

