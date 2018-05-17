import mama

class AlphaGL(mama.BuildTarget):
    local_workspace = 'build'
    def dependencies(self):
        self.add_git('ReCpp', 'https://github.com/RedFox20/ReCpp.git')
        self.add_git('zlib', 'https://github.com/madler/zlib.git')
        self.add_git('libpng', 'https://github.com/kikinteractive/libpng.git')
        self.add_git('libjpeg', 'https://github.com/LuaDist/libjpeg.git')

        self.inject_products('libpng', 'zlib', 'ZLIB_INCLUDE_DIR', 'ZLIB_LIBRARY', 'zlibstatic')
