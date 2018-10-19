# auto_tie
A header-only kinda, sorta emulation of structured binding for C++14

#TLDR

Dependencies: C++14, Boost Preprocessor

`#include <auto_tie.hpp>` in your file. This file is found in include/auto_tie.hpp at https://github.com/jbandela/auto_tie

```cpp
// Set of student id, name, gpa,grade
std::set<std::tuple<int,std::string,char,double>> myset;

// AUTO_TIE copies/moves elements of the tuple/pair
auto r = AUTO_TIE(iterator, success) = myset.insert(std::make_tuple(2,"Raja",'B',3.1)); 
if (r.success) {
  // AUTO_TIE_REF has references to the tuple/pair elements
  auto s = AUTO_TIE_REF(id, name, grade, gpa) = *r.iterator;
  std::cout << "Successfully inserted " << s.id << " " << " " << s.name << " " << s.grade << " " << s.gpa << "\n";
}

```

#Introduction

Bjarne Stroustrup back in Novemeber wrote a nice progress report, available [here](https://isocpp.org/blog/2015/11/kona-standards-meeting-trip-report), of the Kona meeting. One of the proposals considered is called structured binding. The proposal addresses one of the inconveniences of returning multiple values from a function using tuples. While, it is very easy for a function to return multiple values, it is harder for the caller to use them. Here is an example from the write up.

consider the following function

```cpp
tuple<T1,T2,T3> f() { /*...*/ return make_tuple(a,b,c); }

```

If we want to split the tuple into variables without specifying the type, we have to do this;

```cpp
auto t = f();
auto x = get<1>(t);
auto y = get<2>(t);
auto z = get<3>(t);

```

The proposal puts forth the following syntax instead

```cpp
auto {x,y,z} = f();               // x has type T1, y has type T2, z has type T3

```

I am excited for this feature, and for C++17 in general. While waiting for C++17, I decided to see how close I could get with C++14. Here is the result.


```cpp
auto r = AUTO_TIE(x,y,z) = f();               // x has type T1, y has type T2, z has type T3

// Unlike the C++17 feature, you need to use r.x instead of just x
std::cout << r.x << "," << r.y << "," << r.z << "\n";

```

Also, if I have an L-value tuple, and I just want convenient names for the members without moving/copying, I can use `AUTO_TIE_REF`, like this

```
auto t = f();

auto r = AUTO_TIE_REF(x,y,z) = t;

// Can access r.x,r.y,r.z but they are all references to t

```

#Implementation

If you just wanted some background and how to use the library, you can stop reading here. I will now talk about to implement it.

Let us say we have this function

```cpp
template<class T1, class T2, class T3>
std::tuple<T1, T2, T3> f();

```

And we wanted to access the tuple elements as x,y,z.

Here is one way we could do this.

```cpp
template<class T1, class T2, class T3>
struct xyz_elements{
 T1 x;
 T2 y;
 T3 z; 
};

```

Then we can use a helper class to fill in with the tuple values;

```cpp
struct auto_tie_helper{
  
 template<class Tuple}
 auto operator=(Tuple&& t){
   using T = xyz_elements<std::tuple_element_t<0,Tuple>,
     std::tuple_element_t<1,Tuple>,std::tuple_element_t<2,Tuple>>;
   return T{std::get<0>(std::foward<Tuple>(t)),
     std::get<1>(std::foward<Tuple>(t)),std::get<2>(std::foward<Tuple>(t))};

 }

};

template<class T>
auto auto_tie(){return auto_tie_helper{};}

```

Then we can use the above like this

```cpp
auto r = auto_tie() = f();

std::cout << r.x << "\n";

```

This is great... if we only ever wanted to use 3 element tuples and use x,y,z as the element names. Let us make `helper` a template. But what should we take as the template parameter? We would need something like a template template because we do not know types of the tuple elements when we instantiate the helper. However, taking a template template will prove be problematic for reasons that will be explain later. Instead, let us `decltype` with a function object to figure out the types we need.

```cpp
template<class F>
struct auto_tie_helper {

	template<class T, std::size_t... I>
	auto construct(T&& t, std::index_sequence<I...>) {
		using type = decltype(std::declval<F>()(std::get<I>(std::forward<T>(t))...));
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

```

Then we can use `auto_tie` like this.

```cpp
auto r = auto_tie([](auto x, auto y, auto z){return xyz_elements<decltype(x),decltype(y),decltype(z)>{};}) = f();
std::cout << r.x << "\n";

```

The lambda we pass to `auto_tie` returns `xyz_elements` with the correct types. `auto_tie_helper` uses `std::declval` along with `decltype` to get the type that results from calling our lambda (which will be `xyz_elements<decltype(x),decltype(y),decltype(z)>`.

However, what if one of the elements of the tuple is not default constructible? We will get an error in our lambda. To fix this, let us have the lamda return a pointer to `xyz_elements`, and `auto_tie_helper` use `std::remove_pointer_t` to get rid of the pointer. This way, we do not require default construction.

```cpp
template<class F>
struct auto_tie_helper {

	template<class T, std::size_t... I>
	auto construct(T&& t, std::index_sequence<I...>) {
		using type = std::remove_ptr_t<decltype(std::declval<F>()(std::get<I>(std::forward<T>(t))...))>;
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

```

Then we can use `auto_tie` like this.

```cpp

auto r = auto_tie([](auto x, auto y, auto z){return static_cast<xyz_elements<decltype(x),decltype(y),decltype(z)>*>(nullptr);}) = f();
std::cout << r.x << "\n";

```

Now, we can use define a template outside our function for the number of tuple elements we want with the names we want, and use `auto_tie` with that by providng the appropriate lamda function that returns a pointer to the type we want. However, it is still an inconvenience to have to define a template class outside the function where we are using `auto_tie`. However, we cannot define a template class inside a function, as that is forbidden by C++. Instead, we define a class inside our generic lambda what we pass to `auto_tie`.

```cpp

auto r = auto_tie([](auto x_, auto y_, auto z_){
  struct my_struct{
    decltype(x_) x; 
    decltype(y_) y; 
    decltype(z_) z; 

  };
  return static_cast<my_struct*>(nullptr);}) = f();
std::cout << r.x << "\n";

```

By the way, this is the reason that we used a `decltype` with a function object instead of a template template in `auto_tie_helper`. Now we are able to use `auto_tie` in a self-contained way. However, it is very verbose. Because it is self-contained, we can create a macro using Boost Preprocessor to make this all less verbose.

```cpp

#define AUTO_TIE_HELPER1(r, data, i, elem) BOOST_PP_COMMA_IF(i) auto BOOST_PP_CAT(elem,_)
#define AUTO_TIE_HELPER2(r, data, elem) decltype( BOOST_PP_CAT(elem,_) ) elem ;

#define AUTO_TIE_IMPL(seq) auto_tie([]( BOOST_PP_SEQ_FOR_EACH_I(AUTO_TIE_HELPER1, _ , seq ) ) { \
	struct f1f067cb_03fe_47dc_a56d_93407b318d12_auto_tie_struct { BOOST_PP_SEQ_FOR_EACH(AUTO_TIE_HELPER2, _, seq) }; \
    return static_cast<f1f067cb_03fe_47dc_a56d_93407b318d12_auto_tie_struct*>(nullptr);\
})

#define AUTO_TIE(...) AUTO_TIE_IMPL(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) )


```

`AUTO_TIE` takes macro variable args and converts it to a Boost Preprocessor sequence and passes it to `AUTO_TIE_IMPL`. `AUTO_TIE_IMPL` uses `AUTO_TIE_HELPER1` to create the lambda parameters. Then it defines a struct with a unique name so we don't have any accidental name collisions - `f1f067cb_03fe_47dc_a56d_93407b318d12_auto_tie_struct`. Then it uses `AUTO_TIE_HELPER2` to define the members. Finally, as in the hand coded lambda above, it returns a pointer to the struct. `AUTO_TIE_IMPL` calls `auto_tie` with the above lambda. So finally we can write...

```cpp
auto r = AUTO_TIE(x,y,z) = f();
std::cout << r.x << "\n";

```

#Conclusion

I had a lot of fun writing this. I learned the following lessons while doing this.

- C++14 generic lambdas are surprisingly powerful and enable stuff that could not be done before
- By limiting macros to just dealing with names (which templates can't handle), and having templates deal with expressions (which macros are good at messing up), you can get some nice, safe, terse syntax.

I think this technique, can also be extended to do some other cool stuff, that I will discuss in the future.

Let me know what you think.



