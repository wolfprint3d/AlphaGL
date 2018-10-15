import mama
class libjpe(mama.BuildTarget):
    def dependencies(self):
        pass

    def configure(self):
        self.add_cmake_options('BUILD_STATIC=ON', 'BUILD_EXECUTABLES=OFF', 'BUILD_TESTS=OFF')

    def package(self):
        self.export_libs('lib', ['.lib', '.a'])
        self.export_include('include', build_dir=True)