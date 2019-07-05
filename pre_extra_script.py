import os
import shutil

Import("env")

def copy_for_tests(cfg_path):
    with open(cfg_path, "wb") as fout:
        with open(cfg_path + ".default", "rb") as fin:
            for l in fin:
                if l.startswith("#define OWNER "):
                    fout.write('#define OWNER "Tests"\n')
                elif l.startswith("#define NAME "):
                    fout.write('#define NAME "TestsName"\n')
                else:
                    fout.write(l)

cfg_path = os.path.join(env["PROJECTSRC_DIR"], "config.hpp")
if not os.path.isfile(cfg_path):
    if os.environ.get("ROBORUKA_TESTS", "") != "1":
        shutil.copyfile(cfg_path + ".default", cfg_path)
        print("Generated default config at %s!" % cfg_path)
    else:
        copy_for_tests(cfg_path)
        print("Generated config for tests at %s!" % cfg_path)
