mergeInto(LibraryManager.library, {
    tinydomxx_service: function(servicecode, argptr) {
        return tinydomxx_service(servicecode, argptr, HEAPU8.buffer);
    }
});

