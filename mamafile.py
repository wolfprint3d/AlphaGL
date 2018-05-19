import mama
class AlphaGL(mama.BuildTarget):
    local_workspace = 'build'
    def dependencies(self):
        self.add_git('ReCpp',   'https://github.com/RedFox20/ReCpp.git')
        self.add_git('libpng',  'https://github.com/LuaDist/libpng.git')
        self.add_git('libjpeg', 'https://github.com/LuaDist/libjpeg.git')
        self.add_git('glfw',    'https://github.com/glfw/glfw.git')

    def package(self):
        self.export_libs('.', ['.lib', '.a']) # export any .lib or .a from build folder
        self.export_includes(['AGL']) # export AGL as include from source folder

    def test(self):
        pass