#ifndef __ATOMIC_H__
#define __ATOMIC_H__
struct atomic_T{
	__volatile__ long value;
};
inline void atomic_add(struct atomic_T* atomic,long value){
	__asm__ __volatile__(
		"lock addq %1,%0 \n\t"
		:"+m"(atomic->value)
		:"r"(value)
		:"memory"
	);
}
inline void atomic_sub(struct atomic_T* atomic,long value){
	__asm__ __volatile__(
		"lock subq %1,%0 \n\t"
		:"+m"(atomic->value)
		:"r"(value)
		:"memory"
	);
}
inline void atomic_inc(struct atomic_T* atomic){
	__asm__ __volatile__(
		"lock incq %0 \n\t"
		:"+m"(atomic->value)
		:
		:"memory"
	);
}
inline void atomic_dec(struct atomic_T* atomic){
	__asm__ __volatile__(
		"lock decq %0 \n\t"
		:"+m"(atomic->value)
		:
		:"memory"
	);
}
inline void atomic_set_mask(struct atomic_T* atomic,long value){
	__asm__ __volatile__{
		"lock bts %1,%0 \n\t"
		:"+m"(atomic->value)
		:"r"(value)
		:"memory"
	}
}
inline void atomic_clear_mask(struct atomic_T* atomic,long value){
	__asm__ __volatile__{
		"lock btr %1,%0 \n\t"
		:"+m"(atomic->value)
		:"r"(value)
		:"memory"
	}
}
inline long atomic_read(struct atomic_T* atomic){
	long ret;
	__asm__ __volatile__(
		"movq %1,%0 \n\t"
		:"=r"(ret)
		:"m"(atomic->value)
		:"memory"
	);
	return ret;
}
#endif
