#ifndef ISING_CORE_ISING_H_
#define ISING_CORE_ISING_H_

// Ising namespaces.
#ifndef ISING_NAMESPACE_BEGIN
#define ISING_NAMESPACE_BEGIN namespace ising {
#endif
#ifndef ISING_NAMESPACE_END
#define ISING_NAMESPACE_END }
#endif
#ifndef ISING_TOOLKIT_NAMESPACE_BEGIN
#define ISING_TOOLKIT_NAMESPACE_BEGIN namespace ising { namespace toolkit {
#endif
#ifndef ISING_TOOLKIT_NAMESPACE_END
#define ISING_TOOLKIT_NAMESPACE_END } }
#endif

#define ISING_PARALLEL_FOR // #pragma omp parallel for

#endif
