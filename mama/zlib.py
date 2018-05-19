import mama

class zlib_static(mama.BuildTarget):
    def configure(self):
        self.add_cmake_options('BUILD_SHARED_LIB=NO', 'PNG_TESTS=NO')

    def package(self):
        self.export_libs('.', ['zlibstatic.lib', 'libz.a'])
        self.export_includes(['include'], build_dir=True)