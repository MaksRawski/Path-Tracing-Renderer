#ifndef CALL_MACRO_H_
#define CALL_MACRO_H_

void GL_CALL_impl(const char *file_name, int line_num);

#ifndef NDEBUG
#define GL_CALL(_CALL)                                                         \
  _CALL;                                                                       \
  GL_CALL_impl(__FILE__, __LINE__);
#else
#define GL_CALL(_CALL) _CALL
#endif

#endif // CALL_MACRO_H_
