include ../common.mak

LIBJS=$(BIN)/libscriptjs$(SOEXT)
PROGRAM=duk

CUSTOMFLAGS=-I. -I./duk-v1-compat

OBJS=script_js.o duk_alloc_default.o duk_api_buffer.o duk_api_bytecode.o duk_api_call.o duk_api_codec.o duk_api_compile.o duk_api_debug.o duk_api_heap.o duk_api_inspect.o duk_api_memory.o duk_api_object.o duk_api_random.o duk_api_stack.o duk_api_string.o duk_api_time.o duk_bi_array.o duk_bi_boolean.o duk_bi_buffer.o duk_bi_date.o duk_bi_date_unix.o duk_bi_date_windows.o duk_bi_duktape.o duk_bi_encoding.o duk_bi_error.o duk_bi_function.o duk_bi_global.o duk_bi_json.o duk_bi_math.o duk_bi_number.o duk_bi_object.o duk_bi_performance.o duk_bi_pointer.o duk_bi_promise.o duk_bi_proxy.o duk_bi_reflect.o duk_bi_regexp.o duk_bi_string.o duk_bi_symbol.o duk_bi_thread.o duk_bi_thrower.o duk_builtins.o duk_debug_fixedbuffer.o duk_debugger.o duk_debug_macros.o duk_debug_vsnprintf.o duk_error_augment.o duk_error_longjmp.o duk_error_macros.o duk_error_misc.o duk_error_throw.o duk_hbuffer_alloc.o duk_hbuffer_ops.o duk_hbufobj_misc.o duk_heap_alloc.o duk_heap_finalize.o duk_heap_hashstring.o duk_heap_markandsweep.o duk_heap_memory.o duk_heap_misc.o duk_heap_refcount.o duk_heap_stringcache.o duk_heap_stringtable.o duk_hobject_alloc.o duk_hobject_class.o duk_hobject_enum.o duk_hobject_misc.o duk_hobject_pc2line.o duk_hobject_props.o duk_hstring_misc.o duk_hthread_alloc.o duk_hthread_builtins.o duk_hthread_misc.o duk_hthread_stacks.o duk_js_arith.o duk_js_call.o duk_js_compiler.o duk_js_executor.o duk_js_ops.o duk_js_var.o duk_lexer.o duk_numconv.o duk_regexp_compiler.o duk_regexp_executor.o duk_replacements.o duk_selftest.o duk_tval.o duk_unicode_support.o duk_unicode_tables.o duk_util_bitdecoder.o duk_util_bitencoder.o duk_util_bufwriter.o duk_util_cast.o duk_util_double.o duk_util_hashbytes.o duk_util_memory.o duk_util_misc.o duk_util_tinyrandom.o ./duk-v1-compat/duk_v1_compat.o

$(LIBJS): $(OBJS)
	$(CC) -shared -o $(LIBJS) $(OBJS) -L$(BIN) -lpit -lm

$(PROGRAM): duk_cmdline.o $(LIBJS)
	$(CC) -o $(PROGRAM) duk_cmdline.o -L$(BIN) -lpit -lscriptjs -lm

clean:
	rm -f $(LIBJS) $(OBJS) $(PROGRAM)
