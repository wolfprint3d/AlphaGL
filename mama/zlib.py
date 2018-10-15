import mama

class zlib(mama.BuildTarget):
    def configure(self):
        self.add_cmake_options('BUILD_SHARED_LIB=NO', 'PNG_TESTS=NO')

    def package(self):
        self.export_libs('.', ['zlibstatic.lib', 'libz.a'])
        self.export_include('include', build_dir=True)
