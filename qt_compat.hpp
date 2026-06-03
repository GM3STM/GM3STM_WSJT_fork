#ifndef QT_COMPAT_HPP_
#define QT_COMPAT_HPP_

#include <QMetaType>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
template <typename T>
int qRegisterMetaTypeStreamOperators (char const * type_name)
{
  // Qt 6 discovers stream operators automatically, but old settings still
  // depend on these explicitly registered type names.
  return qRegisterMetaType<T> (type_name);
}
#endif

#endif
