import os
import shutil

Import("env")

cfg_path = os.path.join(env["PROJECTSRC_DIR"], "config.hpp")
if not os.path.isfile(cfg_path):
    shutil.copyfile(cfg_path + ".default", cfg_path)
    print("Generated default config at %s!" % cfg_path)
