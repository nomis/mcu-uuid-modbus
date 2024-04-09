#ifndef UUID_MODBUS_MAKE_UNIQUE_CPP_
#define UUID_MODBUS_MAKE_UNIQUE_CPP_

#if !defined(__cpp_lib_make_unique) && !defined(DOXYGEN)

#include <memory>

namespace std {

template<typename _Tp, typename... _Args>
inline unique_ptr<_Tp> make_unique(_Args&&... __args) {
		return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
}

} // namespace std
#endif

#endif
