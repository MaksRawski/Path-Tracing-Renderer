#ifndef CALL_MACRO_H_
#define CALL_MACRO_H_

void GL_CALL_impl(void);

#ifndef NDEBUG
#define GL_CALL(_CALL)                                                         \
  _CALL;                                                                       \
  GL_CALL_impl();
#else
#define GL_CALL(_CALL) _CALL
#endif

#endif // CALL_MACRO_H_
