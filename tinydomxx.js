var tinydomxx_service;
var tinydomxx_ccall;

(
    function () {
        const PTR_SIZE = 8;

        var initial_length = 16;
        var obj_table = new Array(initial_length);
        var next_head = 0;

        var allocated = 0;

        for (var i=0; i<initial_length-1; i++) {
            obj_table[i] = (i+1);
        }

        obj_table[initial_length-1] = -1;

        function resize_table() {
            var new_obj_table = new Array(obj_table.length * 2);
            for (var i=0; i<obj_table.length; i++) {
                new_obj_table[i] = obj_table[i];
            }

            for (var i=obj_table.length; i<new_obj_table.length-1; i++) {
                new_obj_table[i] = (i+1);
            }

            new_obj_table[new_obj_table.length-1] = -1;
            next_head = obj_table.length;

            obj_table = new_obj_table;
        }

        function allocate_entry() {
            if (next_head == -1) {
                resize_table();
            }

            var ret = next_head;
            next_head = obj_table[next_head];
            allocated ++;

            return ret;
        }

        function free_entry(entry) {
            obj_table[entry] = next_head;
            next_head = entry;
            allocated --;
        }

        function ptr_get32(heap, ptr) {
            var view = new Uint32Array(heap);
            return view[ptr>>2];
        }
        function ptr_set32(heap, ptr, value) {
            var view = new Uint32Array(heap);
            view[ptr>>2] = value;
        }

        var to_utf8 = new TextDecoder('utf-8');

        function ptr_get_string(heap, ptr, len) {
            var view = new Uint8Array(heap);
            var subview = view.subarray(ptr, ptr + len);
            return to_utf8.decode(subview);
        }

        var ptr_set_ptr = ptr_set32;
        var ptr_get_ptr = ptr_get32;

        const ALLOC_ENTRY = 0;
        const FREE_ENTRY = 1;
        const DOCUMENT_CREATE_ELEMENT = 2;
        const ELEMENT_SETATTRIBUTE = 3;
        const ELEMENT_APPENDCHILD = 4;
        const DOCUMENT_CREATE_TEXT_NODE = 5;

        function service(servicecode, argptr, heap_buffer) {
            switch (servicecode) {
            case ALLOC_ENTRY:
                var ret = allocate_entry();
                return ret;

            case FREE_ENTRY:
                var value = ptr_get32(heap_buffer, argptr);
                free_entry(value);
                return 0;

            case DOCUMENT_CREATE_ELEMENT:
                var index = allocate_entry();
                var tagptr = ptr_get32(heap_buffer, argptr + 4*0);
                var taglen = ptr_get32(heap_buffer, argptr + 4*1);

                obj_table[index] = document.createElement(ptr_get_string(heap_buffer, tagptr, taglen));
                return index;

            case DOCUMENT_CREATE_TEXT_NODE:
                var index = allocate_entry();
                var strptr = ptr_get32(heap_buffer, argptr + 4*0);
                var strlen = ptr_get32(heap_buffer, argptr + 4*1);

                obj_table[index] = document.createTextNode(ptr_get_string(heap_buffer, strptr, strlen));
                return index;

            case ELEMENT_SETATTRIBUTE:
                var index = ptr_get32(heap_buffer, argptr + 4*0);
                var nameptr = ptr_get32(heap_buffer, argptr + 4*1);
                var namelen = ptr_get32(heap_buffer, argptr + 4*2);
                var valueptr = ptr_get32(heap_buffer, argptr + 4*3);
                var valuelen = ptr_get32(heap_buffer, argptr + 4*4);

                obj_table[index].setAttribute(ptr_get_string(heap_buffer, nameptr, namelen),
                                              ptr_get_string(heap_buffer, valueptr, valuelen));
                return 0;

            case ELEMENT_APPENDCHILD:
                var index = ptr_get32(heap_buffer, argptr + 4*0);
                var child = ptr_get32(heap_buffer, argptr + 4*1);

                obj_table[index].appendChild(obj_table[child]);
                return 0;

            }
        }

        function tccall(c_name, return_type, arg_types, args) {
            var ret_jsobj = false;
            var refcntobj_ptr = 0;
            var jsobj_ptr = 0;
            var objindex = 0;
            var buffer = Module.HEAPU8.buffer;

            var actual_arg_types = new Array(arg_types.length);
            var actual_args = new Array(args.length);

            for (var i in arg_types) {
                if (arg_types[i] == 'jsobj') {
                    refcntobj_ptr = Module._malloc(8);
                    jsobj_ptr = Module._malloc(PTR_SIZE);
                    objindex = allocate_entry();
                    obj_table[objindex] = args[i];

                    ptr_set32(buffer, refcntobj_ptr + 0, 1);
                    ptr_set32(buffer, refcntobj_ptr + 4, objindex);
                    ptr_set_ptr(buffer, jsobj_ptr, refcntobj_ptr);
                    refcntobj_ptr = ptr_get_ptr(buffer, jsobj_ptr);

                    actual_arg_types[i] = "number";
                    actual_args[i] = jsobj_ptr;
                } else {
                    actual_arg_types[i] = arg_types[i];
                    actual_args[i] = args[i];
                }
            }

            var ret = ccall(c_name, return_type, actual_arg_types, actual_args);

            for (var i in arg_types) {
                if (arg_types[i] == 'jsobj') {
                    var jsobj_ptr = actual_args[i];
                    var refcntobj_ptr = ptr_get_ptr(buffer, jsobj_ptr + 0);
                    var refcnt = ptr_get32(buffer, refcntobj_ptr + 0);
                    var objindex = ptr_get32(buffer, refcntobj_ptr + 4);

                    if (refcnt == 1) {
                        Module._free(refcntobj_ptr);
                        free_entry(objindex);
                    } else {
                        ptr_set32(buffer, refcntobj_ptr + 0, refcnt-1);
                    }

                    Module._free(jsobj_ptr);
                }
            }

            return ret;
        }

        function get_alived_obj_count() {
            return allocated;
        }

        tinydomxx_service = service;
        tinydomxx_ccall = tccall;
        tinydomxx_get_alived_obj_count = get_alived_obj_count;
    }

)();
