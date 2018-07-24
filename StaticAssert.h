#pragma once

#define _impl_PASTE(a,b) a##b

#define _impl_CASSERT_LINE(predicate, line, file) \
    typedef char _impl_PASTE(assertion_failed_##file##_,line)[(predicate) ? 1 : -1];

#define CASSERT(predicate, file) _impl_CASSERT_LINE(predicate, __LINE__, file)