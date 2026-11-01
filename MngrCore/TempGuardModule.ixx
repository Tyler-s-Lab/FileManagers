export module TempGuardModule;

import <type_traits>;

/**
 * @brief 临时获取所有权，并保护目标的旧有值，析构时自动还原旧值。
 * @tparam T: 可无异常复制/赋值的非 const、非引用类型。
*/
export 
template<class T>
class TempGuard final {
	static_assert(!std::is_reference_v<T> && !std::is_const_v<T>,
		"TempGuard requires a non-const, non-reference value type.");
	static_assert(std::is_nothrow_copy_constructible_v<T>,
		"TempGuard requires a nothrow copy-constructible type.");
	static_assert(std::is_nothrow_copy_assignable_v<T>,
		"TempGuard requires a nothrow copy-assignable type.");
public:
	/**
	 * @brief 构造函数，自动复制保留旧值。
	 * @param t: 目标对象。
	*/
	explicit TempGuard(T& t) noexcept :
		r_target(t), m_old(t) {
	}
	/**
	 * @brief 析构函数，自动还原旧值。
	*/
	~TempGuard() noexcept {
		r_target = m_old;
	}

	TempGuard(const TempGuard&) = delete;
	TempGuard& operator=(const TempGuard&) = delete;

	/**
	 * @brief 临时修改。把目标的值变为n。
	 * @param n: 临时值。
	*/
	void operator=(const T& n) noexcept {
		r_target = n;
	}
	/**
	 * @brief 临时修改。把目标的值变为n。
	 * @param n: 临时值。
	*/
	void operator=(T&& n) noexcept {
		r_target = n;
	}

protected:
	T& r_target;
	T m_old;
}; // template class TempGuard
