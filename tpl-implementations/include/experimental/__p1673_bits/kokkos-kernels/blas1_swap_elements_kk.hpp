
#ifndef LINALG_TPLIMPLEMENTATIONS_INCLUDE_EXPERIMENTAL___P1673_BITS_KOKKOSKERNELS_SWAP_HPP_
#define LINALG_TPLIMPLEMENTATIONS_INCLUDE_EXPERIMENTAL___P1673_BITS_KOKKOSKERNELS_SWAP_HPP_

#include <utility>

namespace KokkosKernelsSTD {

// this is here until we can use kokkos 3.6 which has swap avail
template <class T>
KOKKOS_INLINE_FUNCTION void _my_tmp_swap(T& a, T& b) noexcept {
  static_assert(
      std::is_move_assignable<T>::value &&
      std::is_move_constructible<T>::value,
      "KokkosKernelsSTD::swap arguments must be move assignable and move constructible");

  T tmp = std::move(a);
  a     = std::move(b);
  b     = std::move(tmp);
}

//
// for now, specialize for default_accessor
// https://github.com/kokkos/stdBLAS/issues/122
//
template<class ExeSpace,
	 class ElementType_x,
         std::experimental::extents<>::size_type ... ext_x,
         class Layout_x,
         class ElementType_y,
         std::experimental::extents<>::size_type ... ext_y,
         class Layout_y>
  requires (sizeof...(ext_x) == sizeof...(ext_y))
void swap_elements(kokkos_exec<ExeSpace> /*kexe*/,
		   std::experimental::mdspan<
		     ElementType_x,
		     std::experimental::extents<ext_x ...>,
		     Layout_x,
		     std::experimental::default_accessor<ElementType_x>
		   > x,
		   std::experimental::mdspan<
		     ElementType_y,
		     std::experimental::extents<ext_y ...>,
		     Layout_y,
		     std::experimental::default_accessor<ElementType_y>
		   > y)
{
  // constraints
  // matching rank already checked via requires above
  static_assert(x.rank() <= 2);

  // preconditions
  if ( x.extent(0) != y.extent(0) ){
    throw std::runtime_error("KokkosBlas: swap_elements: x.extent(0) != y.extent(0) ");
  }

  if constexpr(x.rank()==2){
    if ( x.extent(1) != y.extent(1) ){
      throw std::runtime_error("KokkosBlas: swap_elements: x.extent(1) != y.extent(1) ");
    }
  }

  // this print is detected in the tests
#if defined KOKKOS_STDBLAS_ENABLE_TESTS
  std::cout << "swap_elements: kokkos impl\n";
#endif

  auto x_view = Impl::mdspan_to_view(x);
  auto y_view = Impl::mdspan_to_view(y);

  auto ex = ExeSpace();
  if constexpr(x.rank()==1){
    Kokkos::parallel_for(Kokkos::RangePolicy(ex, 0, x_view.extent(0)),
			 KOKKOS_LAMBDA (const std::size_t & i){
			   _my_tmp_swap(x_view(i), y_view(i));
			 });
  }

  else{
    Kokkos::parallel_for(Kokkos::RangePolicy(ex, 0, x_view.extent(0)),
			 KOKKOS_LAMBDA (const std::size_t & i){
			   for (std::size_t j=0; j<x_view.extent(1); ++j){
			     _my_tmp_swap(x_view(i,j), y_view(i,j));
			   }
			 });
  }

  //fence message when using latest kokkos:
  ex.fence();
  // ex.fence("KokkosStdBlas::swap_elements: fence after operation");
}

} // end namespace KokkosKernelsSTD
#endif
