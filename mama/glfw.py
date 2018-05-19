import mama
class glfw(mama.BuildTarget):
    def dependencies(self):
        pass

    def configure(self):
        self.add_cmake_options('GLFW_BUILD_EXAMPLES=NO')
        self.add_cmake_options('GLFW_BUILD_TESTS=NO')
        self.add_cmake_options('GLFW_BUILD_DOCS=NO')
        self.add_cmake_options('GLFW_BUILD_EXAMPLES=NO')
        self.add_cmake_options('USE_MSVC_RUNTIME_LIBRARY_DLL=NO')

    def package(self):
        self.export_libs('lib', ['.lib', '.a'])
        self.export_include('include', build_dir=True)
    