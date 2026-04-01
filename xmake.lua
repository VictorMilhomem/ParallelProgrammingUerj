add_rules("mode.debug", "mode.release")

add_requires("openmp")

target("ParallelProgrammingUerj")
    set_kind("binary")
    set_languages("c23")
    add_files("src/*.c")
    add_packages("openmp")


