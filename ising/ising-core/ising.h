#ifndef ISING_CORE_ISING_H_
#define ISING_CORE_ISING_H_

// Ising namespaces.
#define ISING_NAMESPACE_BEGIN         namespace ising {
#define ISING_NAMESPACE_END           }
#define ISING_TOOLKIT_NAMESPACE_BEGIN namespace ising { namespace toolkit {
#define ISING_TOOLKIT_NAMESPACE_END   } }
#define ISING_EXACT_NAMESPACE_BEGIN   namespace ising { namespace exact {
#define ISING_EXACT_NAMESPACE_END     } }

// Use OpenMP for parallel speedup.
#ifndef _DEBUG
#define ISING_PARALLEL
#endif
// Use pre-evaluated valued to replace `exp()`.
#define ISING_FAST_EXP

#endif
