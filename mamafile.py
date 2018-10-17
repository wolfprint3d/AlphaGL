import mama
class AlphaGL(mama.BuildTarget):
    local_workspace = 'build'
    def dependencies(self):
        if self.linux:
            self.add_git('libpng', 'https://github.com/glennrp/libpng.git', mamafile='mama/libpng.py')
        else:
            self.add_git('libpng', 'https://github.com/LuaDist/libpng.git', mamafile='mama/libpng.py')
        self.add_git('libjpeg', 'https://github.com/LuaDist/libjpeg.git', mamafile='mama/libjpeg.py')
        self.add_git('glfw',    'https://github.com/glfw/glfw.git', mamafile='mama/glfw.py')
        self.add_git('ReCpp',   'https://github.com/RedFox20/ReCpp.git')

    def package(self):
        self.export_libs('.', ['.lib', '.a']) # export any .lib or .a from build folder
        self.export_include('.')
        if self.linux:
            self.export_syslib('GL')
            self.export_syslib('X11')

    def test(self, args):
        self.gdb('bin/AGLTests')