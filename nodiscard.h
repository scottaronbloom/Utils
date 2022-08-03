#ifndef __NODISCARD_H
#define __NODISCARD_H

#ifdef Q_OS_WINDOWS
#define NODISCARD [[nodiscard]]
#else
#define NODISCARD 
#endif

#endif
