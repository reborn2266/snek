/*
 * Copyright © 2018 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "snek-builtin.h"

#ifndef SNEK_DEBUG
#define SNEK_DEBUG	1
#endif

// #define DEBUG_MEMORY

#ifdef DEBUG_MEMORY
#define debug_memory(fmt, args...) printf(fmt, ## args)
#else
#define debug_memory(fmt, args...)
#endif

#ifndef SNEK_POOL
#define SNEK_POOL		(12 * 256)
#endif
#define SNEK_POOL_EXTRA		0
#define SNEK_ALLOC_SHIFT	2
#define SNEK_ALLOC_ROUND	(1 << SNEK_ALLOC_SHIFT)
#define SNEK_OFFSET_MASK	0x00fffffc

#if SNEK_POOL <= 65536
typedef uint16_t	snek_offset_t;
typedef int16_t		snek_soffset_t;
#define SNEK_OFFSET_NONE	0xffffu
#define SNEK_SOFFSET_NONE	0x7fff
#else
typedef uint32_t	snek_offset_t;
typedef int32_t		snek_soffset_t;
#define SNEK_OFFSET_NONE	0xffffffffu
#define SNEK_SOFFSET_NONE	0x7fffffff
#endif

typedef snek_offset_t snek_id_t;

#define SNEK_ID_NONE		0

typedef union {
	uint32_t    u;
	float       f;
} snek_poly_t;

typedef enum {
	snek_op_plus = 0,
	snek_op_minus = 1,
	snek_op_times = 2,
	snek_op_divide = 3,
	snek_op_div = 4,
	snek_op_mod = 5,
	snek_op_pow = 6,
	snek_op_land = 7,
	snek_op_lor = 8,
	snek_op_lxor = 9,
	snek_op_lshift = 10,
	snek_op_rshift = 11,

	snek_op_assign_plus = 12,
	snek_op_assign_minus = 13,
	snek_op_assign_times = 14,
	snek_op_assign_divide = 15,
	snek_op_assign_div = 16,
	snek_op_assign_mod = 17,
	snek_op_assign_pow = 18,
	snek_op_assign_land = 19,
	snek_op_assign_lor = 20,
	snek_op_assign_lxor = 21,
	snek_op_assign_lshift = 22,
	snek_op_assign_rshift = 23,

	snek_op_num,
	snek_op_int,
	snek_op_string,
	snek_op_list,
	snek_op_tuple,
	snek_op_id,

	snek_op_not,

	snek_op_eq,
	snek_op_ne,
	snek_op_gt,
	snek_op_lt,
	snek_op_ge,
	snek_op_le,

	snek_op_is,
	snek_op_is_not,
	snek_op_in,
	snek_op_not_in,

	snek_op_uminus,
	snek_op_lnot,

	snek_op_call,

	snek_op_array,
	snek_op_slice,

	snek_op_assign,
	snek_op_assign_named,

	snek_op_global,

	snek_op_branch,
	snek_op_branch_true,
	snek_op_branch_false,
	snek_op_forward,
	snek_op_range_start,
	snek_op_range_step,
	snek_op_in_start,
	snek_op_in_step,

	snek_op_nop,

	snek_op_line,

	snek_op_push = 0x80,
} __attribute__((packed)) snek_op_t;

typedef enum {
	snek_forward_return,
	snek_forward_break,
	snek_forward_continue,
	snek_forward_if,
} __attribute__((packed)) snek_forward_t;

typedef enum {
	snek_list = 0,
	snek_string = 1,
	snek_func = 2,
	snek_builtin = 3,
	snek_float = 4,
} __attribute__((packed)) snek_type_t;

typedef struct snek_mem {
	snek_offset_t	(*size)(void *addr);
	void		(*mark)(void *addr);
	void		(*move)(void *addr);
#ifdef SNEK_MEM_INCLUDE_NAME
	char		name[];
#endif
} snek_mem_t;

#ifndef SNEK_MEM_DECLARE
#define SNEK_MEM_DECLARE(n) n
#define SNEK_MEM_SIZE(m)	((m)->size)
#define SNEK_MEM_MARK(m)	((m)->mark)
#define SNEK_MEM_MOVE(m)	((m)->move)
#endif

#ifdef SNEK_MEM_INCLUDE_NAME
#define SNEK_MEM_DECLARE_NAME(_name)	.name = _name,
#else
#define SNEK_MEM_DECLARE_NAME(_name)
#endif

typedef struct snek_list {
	snek_offset_t	size;
	snek_offset_t	alloc;
	snek_offset_t	note_next_and_readonly;
	snek_offset_t	data;
} snek_list_t;

typedef struct snek_code {
	snek_offset_t	size;
	uint8_t		code[0];
} snek_code_t;

typedef struct snek_range {
	snek_offset_t	prev;
	snek_id_t	id;
	float		current;
	float		limit;
	float		step;
} snek_range_t;

typedef struct snek_in {
	snek_offset_t	prev;
	snek_id_t	id;
	snek_poly_t	array;
	snek_offset_t	i;
} snek_in_t;

typedef struct snek_func {
	snek_soffset_t	nformal;
	snek_offset_t	code;
	snek_id_t	formals[0];
} snek_func_t;

#define SNEK_FUNC_VARARGS	SNEK_SOFFSET_NONE

typedef struct snek_name {
	snek_offset_t	next;
	char		name[0];
} snek_name_t;

typedef struct snek_variable {
	snek_id_t	id;
	snek_poly_t	value;
} snek_variable_t;

typedef struct snek_frame {
	snek_offset_t	prev;
	snek_offset_t	code;
	snek_offset_t	ip;
	snek_offset_t	nvariables;
	snek_variable_t	variables[0];
} snek_frame_t;


typedef struct snek_slice {
	/* provided parameters */
	int32_t		start;
	int32_t		end;
	int32_t		stride;

	/* provided length of object */
	snek_offset_t	len;

	/* computed number of outputs */
	snek_offset_t	count;

	/* computed position of current object */
	snek_offset_t	pos;
} snek_slice_t;

#define SNEK_SLICE_DEFAULT	0x7fffffff	/* empty value provided [1:] */

typedef struct snek_builtin {
	int8_t nformal;
	union {
		snek_poly_t	(*funcv)(uint8_t nposition, uint8_t nnamed,
					 snek_poly_t *args);
		snek_poly_t	(*func0)(void);
		snek_poly_t	(*func1)(snek_poly_t a0);
		snek_poly_t	(*func2)(snek_poly_t a0, snek_poly_t a1);
		snek_poly_t	(*func3)(snek_poly_t a0, snek_poly_t a1, snek_poly_t a2);
		snek_poly_t	(*func4)(snek_poly_t a0, snek_poly_t a1, snek_poly_t a2, snek_poly_t a3);
	};
} snek_builtin_t;

typedef struct snek_buf {
	int	(*put_c)(int c, void *closure);
	int	(*put_s)(const char *s, void *closure);
	void	*closure;
} snek_buf_t;

extern const snek_builtin_t snek_builtins[];

#define SNEK_BUILTIN_VARARGS	-1

#define SNEK_NAN_U	0x7fffffffu
#define SNEK_NAN	((snek_poly_t) { .u = SNEK_NAN_U })
#define SNEK_NULL_U	0xffffffffu
#define SNEK_NULL	((snek_poly_t) { .u = SNEK_NULL_U })
#define SNEK_GLOBAL_U	0xfffffffeu
#define SNEK_GLOBAL	((snek_poly_t) { .u = SNEK_GLOBAL_U })
#define SNEK_ZERO	((snek_poly_t) { .f = 0.0f })
#define SNEK_ONE	((snek_poly_t) { .f = 1.0f })

#ifndef SNEK_STACK
#define SNEK_STACK	256
#endif
extern snek_poly_t	snek_stack[SNEK_STACK];
extern snek_offset_t	snek_stackp;

static inline bool
snek_is_nan(snek_poly_t p)
{
	return p.u == SNEK_NAN_U;
}

static inline bool
snek_is_null(snek_poly_t p)
{
	return p.u == SNEK_NULL_U;
}

static inline bool
snek_is_global(snek_poly_t p)
{
	return p.u == SNEK_GLOBAL_U;
}


#ifdef SNEK_DYNAMIC
extern uint8_t *snek_pool  __attribute__((aligned(SNEK_ALLOC_ROUND)));
extern uint32_t	snek_pool_size;
#else
extern uint8_t	snek_pool[SNEK_POOL + SNEK_POOL_EXTRA] __attribute__((aligned(SNEK_ALLOC_ROUND)));
#endif

#include "snek-gram.h"

typedef union {
	bool		bools;
	int16_t		_ints;
	uint8_t		indent;
	snek_offset_t	line;
	snek_op_t	op;
	snek_offset_t	offset;
	snek_soffset_t	soffset;
	snek_id_t	id;
	float		number;
	char		*string;
} snek_token_val_t;

extern snek_token_val_t	snek_token_val;

/* snek-builtin.c */

#define SNEK_BUILTIN_DECLS
#include "snek-builtin.h"

/* snek-code.c */

extern const char * const snek_op_names[];

extern uint8_t		*snek_compile;
extern snek_offset_t	snek_compile_size;
extern snek_offset_t	snek_compile_prev, snek_compile_prev_prev;

#define SNEK_OP_SLICE_START	1
#define SNEK_OP_SLICE_END	2
#define SNEK_OP_SLICE_STRIDE	4

void
snek_code_add_op(snek_op_t op);

void
snek_code_add_op_id(snek_op_t op, snek_id_t id);

void
snek_code_add_op_array(snek_op_t op);

void
snek_code_add_number(float number);

void
snek_code_add_string(char *string);

void
snek_code_add_op_offset(snek_op_t op, snek_offset_t offset);

void
snek_code_add_op_branch(snek_op_t op);

void
snek_code_add_forward(snek_forward_t forward);

void
snek_code_patch_forward(snek_offset_t start, snek_forward_t forward, snek_offset_t target);

void
snek_code_add_slice(bool has_start, bool has_end, bool has_stride);

void
snek_code_add_range_start(snek_id_t id, snek_offset_t nactual);

void
snek_code_patch_branch(snek_offset_t branch, snek_offset_t target);

snek_code_t *
snek_code_finish(void);

snek_offset_t
snek_code_line(snek_code_t *code);

void
snek_run_mark(void);

void
snek_run_move(void);

snek_poly_t
snek_code_run(snek_code_t *code);

void
snek_undefined(snek_id_t id);

extern const snek_mem_t snek_code_mem;
extern const snek_mem_t snek_compile_mem;

/* snek-error.c */

#ifndef snek_error_name
#define snek_error_name snek_error
#endif

void
snek_error_name(const char *format, ...);

#if SNEK_DEBUG
void
snek_panic(const char *message);
#endif

#ifndef sprintf_const
#define sprintf_const sprintf
#endif

extern bool snek_abort;

/* snek-for.c */

void
snek_range_start(snek_id_t id, snek_offset_t nparam);

bool
snek_range_step(void);

extern const snek_mem_t snek_range_mem;
extern snek_range_t *snek_ranges;

void
snek_in_start(snek_id_t id);

bool
snek_in_step(void);

extern const snek_mem_t snek_in_mem;
extern snek_in_t *snek_ins;

/* snek-frame.c */

extern snek_frame_t	*snek_globals;
extern snek_frame_t	*snek_frame;

extern const snek_mem_t snek_variable_mem;

bool
snek_frame_mark_global(snek_offset_t name);

snek_frame_t *
snek_frame_push(snek_code_t *code, snek_offset_t ip, snek_offset_t nformal);

snek_code_t *
snek_frame_pop(snek_offset_t *ip_p);

snek_poly_t *
snek_id_ref(snek_id_t id, bool insert);

bool
snek_id_is_local(snek_id_t id);

bool
snek_id_del(snek_id_t id);

extern const snek_mem_t snek_frame_mem;

/* snek-func.c */

snek_func_t *
snek_func_alloc(snek_code_t *code, snek_offset_t nparam, snek_id_t *formals);

bool
snek_func_push(snek_func_t *func, uint8_t nposition, uint8_t nnamed, snek_code_t *code, snek_offset_t ip);

snek_code_t *
snek_func_pop(snek_offset_t *ip);

extern const snek_mem_t snek_func_mem;

/* snek-gram.y */

extern bool snek_print_vals;

/* snek-lex.l */

extern uint8_t snek_current_indent;
extern char *snek_file;
extern snek_offset_t snek_line;
extern uint8_t snek_ignore_nl;
extern char snek_lex_text[];

/* snek-list.c */

snek_list_t *
snek_list_make(snek_offset_t size, bool readonly);

snek_list_t *
snek_list_append(snek_list_t *list, snek_list_t *append);

snek_list_t *
snek_list_plus(snek_list_t *a, snek_list_t *b);

snek_list_t *
snek_list_times(snek_list_t *a, snek_soffset_t count);

bool
snek_list_equal(snek_list_t *a, snek_list_t *b);

snek_poly_t
snek_list_imm(snek_offset_t size, bool readonly);

snek_list_t *
snek_list_slice(snek_list_t *list, snek_slice_t *slice);

extern const snek_mem_t snek_list_mem;

/* snek-lex.c */

extern uint8_t snek_lex_indent;

token_t
snek_lex(void);

/* snek-memory.c */

#define SNEK_COLLECT_FULL		0
#define SNEK_COLLECT_INCREMENTAL	1

bool
snek_poly_mark(snek_poly_t p);

bool
snek_poly_mark_ref(snek_poly_t *p);

snek_offset_t
snek_collect(uint8_t style);

bool
snek_mark_blob(void *addr, snek_offset_t size);

bool
snek_mark_block_addr(const struct snek_mem *type, void *addr);

bool
snek_mark_block_offset(const struct snek_mem *type, snek_offset_t offset);

bool
snek_mark_addr(const struct snek_mem *type, void *addr);

bool
snek_mark_offset(const struct snek_mem *type, snek_offset_t offset);

bool
snek_move_block_offset(snek_offset_t *ref);

bool
snek_move_block_addr(void **ref);

bool
snek_poly_move(snek_poly_t *ref);

bool
snek_move_addr(const struct snek_mem *type, void **ref);

bool
snek_move_offset(const struct snek_mem *type, snek_offset_t *ref);

void *
snek_alloc(snek_offset_t size);

void
snek_poly_stash(snek_poly_t p);

snek_poly_t
snek_poly_fetch(void);

void
snek_string_stash(const char *s);

char *
snek_string_fetch(void);

void
snek_list_stash(snek_list_t *list);

snek_list_t *
snek_list_fetch(void);

void
snek_code_stash(snek_code_t *code);

snek_code_t *
snek_code_fetch(void);

/* snek-name.c */

snek_id_t
snek_name_id(char *name, bool *keyword);

const char *
snek_name_string(snek_id_t id);

extern const snek_mem_t snek_name_mem;
extern snek_name_t *snek_names;

/* snek-parse.c */

typedef enum {
	snek_parse_success,
	snek_parse_error,
} snek_parse_ret_t;

snek_parse_ret_t
snek_parse(void);

/* snek-poly.c */

void *
snek_ref(snek_poly_t poly);

snek_poly_t
snek_poly_offset(snek_offset_t offset, snek_type_t type);

snek_poly_t
snek_poly(const void *addr, snek_type_t type);

void
snek_poly_print(FILE *file, snek_poly_t poly, char format);

bool
snek_poly_equal(snek_poly_t a, snek_poly_t b);

bool
snek_poly_true(snek_poly_t a);

int
snek_poly_len(snek_poly_t a);

snek_offset_t
snek_null_size(void *addr);

void
snek_null_mark_move(void *addr);

extern const snek_mem_t snek_null_mem;

void
snek_poly_format(snek_buf_t *buf, snek_poly_t a, char format);

/* snek-print.c */
void
snek_print(snek_buf_t *buf, snek_poly_t poly);

/* snek-string.c */

char *
snek_string_make(char c);

char
snek_string_get(char *string, int i);

char *
snek_string_cat(char *a, char *b);

char *
snek_string_slice(char *a, snek_slice_t *slice);

char *
snek_string_interpolate(char *a, snek_poly_t poly);

extern const snek_mem_t snek_string_mem;

/* inlines */

static inline void
snek_stack_push(snek_poly_t p)
{
	if (snek_stackp == SNEK_STACK) {
		snek_error("stack overflow");
		return;
	}
	snek_stack[snek_stackp++] = p;
}

static inline snek_poly_t
snek_stack_pop(void)
{
#if SNEK_DEBUG
	if (!snek_stackp)
		snek_panic("stack underflow");
#endif
	return snek_stack[--snek_stackp];
}

static inline snek_poly_t
snek_stack_pick(snek_offset_t off)
{
	return snek_stack[snek_stackp - off - 1];
}

static inline void
snek_stack_drop(snek_offset_t off)
{
	snek_stackp -= off;
}

snek_soffset_t
snek_stack_pop_soffset(void);

float
snek_stack_pop_float(void);

static inline void
snek_slice_start(snek_slice_t *slice)
{
	slice->pos = slice->start;
}

static inline bool
snek_slice_test(snek_slice_t *slice)
{
	return slice->stride > 0 ? slice->pos < slice->end : slice->pos > slice->end;
}

static inline void
snek_slice_step(snek_slice_t *slice)
{
	slice->pos += slice->stride;
}

static inline bool
snek_slice_identity(snek_slice_t *slice)
{
	return slice->start == 0 && slice->end == slice->len && slice->stride == 1;
}

static inline void *
snek_pool_ref(snek_offset_t offset)
{
	if (offset == 0)
		return NULL;

#if SNEK_DEBUG
	if (((offset - 1) & (SNEK_ALLOC_ROUND-1)) != 0)
		snek_panic("bad offset");
#endif

	return snek_pool + offset - 1;
}

static inline snek_offset_t
snek_pool_offset(const void *addr)
{
	if (addr == NULL)
		return 0;

#if SNEK_DEBUG
	if (((uintptr_t) addr & (SNEK_ALLOC_ROUND-1)) != 0)
		snek_panic("bad address");
#endif

	return ((const uint8_t *) addr) - snek_pool + 1;
}

static inline bool
snek_is_float(snek_poly_t v)
{
	if ((v.u & 0xff000000) != 0xff000000 || v.u == SNEK_NAN_U)
		return true;
	return false;
}

static inline snek_poly_t
snek_float_to_poly(float f)
{
	if (isnanf(f))
		return SNEK_NAN;
	return (snek_poly_t) { .f = f };
}

static inline snek_poly_t
snek_offset_to_poly(snek_offset_t offset, snek_type_t type)
{
	return (snek_poly_t) { .u = 0xff000000 | offset | type };
}

static inline snek_offset_t
snek_poly_to_offset(snek_poly_t v)
{
	return v.u & SNEK_OFFSET_MASK;
}

static inline float
snek_poly_to_float(snek_poly_t v)
{
	return v.f;
}

static inline int
snek_is_pool_addr(const void *addr) {
	const uint8_t *a = addr;
	return (snek_pool <= a) && (a < snek_pool + SNEK_POOL);
}

static inline snek_type_t
snek_poly_type(snek_poly_t v)
{
	return snek_is_float(v) ? snek_float : (v.u & 3);
}

static inline int
snek_size_round(int size)
{
	return (size + (SNEK_ALLOC_ROUND - 1)) & ~(SNEK_ALLOC_ROUND - 1);
}

static inline int
snek_size(const snek_mem_t *mem, void *addr)
{
	return snek_size_round(SNEK_MEM_SIZE(mem)(addr));
}

static inline snek_poly_t
snek_list_to_poly(snek_list_t *list)
{
	return snek_poly(list, snek_list);
}

static inline snek_list_t *
snek_poly_to_list(snek_poly_t poly)
{
	return snek_ref(poly);
}

static inline snek_poly_t *
snek_list_data(snek_list_t *list)
{
	return snek_pool_ref(list->data);
}

static inline snek_poly_t
snek_string_to_poly(char *string)
{
	return snek_poly(string, snek_string);
}

static inline char *
snek_poly_to_string(snek_poly_t poly)
{
	return snek_ref(poly);
}

static inline snek_func_t *
snek_poly_to_func(snek_poly_t poly)
{
	return snek_ref(poly);
}

static inline snek_poly_t
snek_func_to_poly(snek_func_t *func)
{
	return snek_poly(func, snek_func);
}

static inline snek_poly_t
snek_builtin_id_to_poly(snek_id_t id)
{
	return snek_offset_to_poly(id << SNEK_ALLOC_SHIFT, snek_builtin);
}

static inline snek_id_t
snek_poly_to_builtin_id(snek_poly_t a)
{
	return snek_poly_to_offset(a) >> SNEK_ALLOC_SHIFT;
}

static inline const snek_builtin_t *
snek_poly_to_builtin(snek_poly_t a)
{
	return snek_builtins + (snek_poly_to_builtin_id(a) - 1);
}

static inline snek_poly_t
snek_bool_to_poly(bool b)
{
	return b ? SNEK_ONE : SNEK_ZERO;
}

static inline snek_offset_t
snek_offset_value(snek_offset_t offset)
{
	offset = offset & ~3;
	if (offset)
		offset -= 3;
	return offset;
}

static inline bool
snek_offset_flag_0(snek_offset_t offset)
{
	return offset & 1;
}

static inline bool
snek_offset_flag_1(snek_offset_t offset)
{
	return !!(offset & 2);
}

static inline snek_offset_t
snek_offset_set_flag_0(snek_offset_t offset, bool flag)
{
	return (offset & ~1) | (flag ? 1 : 0);
}

static inline snek_offset_t
snek_offset_set_flag_1(snek_offset_t offset, bool flag)
{
	return (offset & ~2) | (flag ? 2 : 0);
}

static inline snek_offset_t
snek_offset_set_value(snek_offset_t offset, snek_offset_t value)
{
	if (value)
		value += 3;
#if SNEK_DEBUG
	if (value & 3)
		snek_panic("note_next bad alignment");
#endif
	return value | (offset & 3);
}

static inline bool
snek_list_readonly(snek_list_t *list)
{
	return snek_offset_flag_0(list->note_next_and_readonly);
}

static inline bool
snek_list_noted(snek_list_t *list)
{
	return snek_offset_flag_1(list->note_next_and_readonly);
}

static inline snek_offset_t
snek_list_note_next(snek_list_t *list)
{
	return snek_offset_value(list->note_next_and_readonly);
}

static inline void
snek_list_set_readonly(snek_list_t *list, bool readonly)
{
	list->note_next_and_readonly = snek_offset_set_flag_0(list->note_next_and_readonly, readonly);
}

static inline void
snek_list_set_noted(snek_list_t *list, bool noted)
{
	list->note_next_and_readonly = snek_offset_set_flag_1(list->note_next_and_readonly, noted);
}

static inline void
snek_list_set_note_next(snek_list_t *list, snek_offset_t note_next)
{
	list->note_next_and_readonly = snek_offset_set_value(list->note_next_and_readonly, note_next);
}

static inline snek_offset_t
snek_code_current(void)
{
	return snek_compile_size;
}

static inline snek_offset_t
snek_code_prev_insn(void)
{
	return snek_compile_prev;
}

static inline snek_offset_t
snek_code_prev_prev_insn(void)
{
	return snek_compile_prev_prev;
}

static inline uint8_t *
snek_code_at(snek_offset_t offset)
{
	return snek_compile + offset;
}

static inline void
snek_code_set_push(snek_offset_t offset)
{
	snek_compile[offset] |= snek_op_push;
}

static inline void
snek_code_delete_prev(void)
{
	snek_compile_size = snek_compile_prev;
	snek_compile_prev = snek_compile_prev_prev;
}