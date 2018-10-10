import mama
class libpng_static(mama.BuildTarget):
    def dependencies(self):
        self.add_git('zlib', 'https://github.com/madler/zlib.git', mamafile='zlib.py')

    def configure(self):
        zinclude, zlibrary = self.get_target_products('zlib')
        self.add_cmake_options(f'ZLIB_INCLUDE_DIR={zinclude}')
        self.add_cmake_options(f'ZLIB_LIBRARY={zlibrary}')
        self.add_cmake_options('BUILD_SHARED_LIB=NO', 'PNG_TESTS=NO', 'PNG_DEBUG=NO')

    def package(self):
        self.export_libs('lib', ['.lib', '.a'])
        self.export_include('include', build_dir=True)