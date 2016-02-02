#pragma once
#include <utility>
#include <tuple>

template<class F>
struct auto_tie_helper {

	template<class T, std::size_t... I>
	auto construct(T&& t, std::index_sequence<I...>) {
		using type = std::remove_pointer_t<decltype(std::declval<F>()(std::get<I>(std::forward<T>(t))...))>;
		return type{ std::get<I>(std::forward<T>(t))... };
	}
	template<class T>
	auto operator=(T&& t) {
		return construct(std::forward<T>(t), std::make_index_sequence<std::tuple_size<std::decay_t<T>>::value>{});
	}

};

template<class F>
auto auto_tie(F f) {
	return auto_tie_helper<F>{};
}

#include <boost/preprocessor.hpp>

#define AUTO_TIE_HELPER1(r, data, i, elem) BOOST_PP_COMMA_IF(i) auto BOOST_PP_CAT(elem,_)
#define AUTO_TIE_HELPER2(r, data, elem) decltype( BOOST_PP_CAT(elem,_) ) elem ;

#define AUTO_TIE_IMPL(seq) auto_tie([]( BOOST_PP_SEQ_FOR_EACH_I(AUTO_TIE_HELPER1, _ , seq ) ) { \
	struct f1f067cb_03fe_47dc_a56d_93407b318d12_auto_tie_struct { BOOST_PP_SEQ_FOR_EACH(AUTO_TIE_HELPER2, _, seq) }; \
    return static_cast<f1f067cb_03fe_47dc_a56d_93407b318d12_auto_tie_struct*>(nullptr);\
})

#define AUTO_TIE(...) AUTO_TIE_IMPL(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) )

#define AUTO_TIE_REF_HELPER1(r, data, i, elem) BOOST_PP_COMMA_IF(i)  auto&& BOOST_PP_CAT(elem,data)
#define AUTO_TIE_REF_HELPER2(r, data, elem) static_assert(!std::is_rvalue_reference<decltype( BOOST_PP_CAT(elem,data) )>::value ,"Cannot use AUTO_TIE_REF with rvalue"); decltype( BOOST_PP_CAT(elem,data) ) elem ;

#define AUTO_TIE_REF_IMPL(seq) auto_tie([]( BOOST_PP_SEQ_FOR_EACH_I(AUTO_TIE_REF_HELPER1, _, seq )  ) { \
	struct f1f067cb_03fe_47dc_a56d_93407b318d12_auto_tie_struct { BOOST_PP_SEQ_FOR_EACH(AUTO_TIE_REF_HELPER2, _, seq )}; \
    return static_cast<f1f067cb_03fe_47dc_a56d_93407b318d12_auto_tie_struct*>(nullptr);\
})

#define AUTO_TIE_REF(...) AUTO_TIE_REF_IMPL( BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) ) 

