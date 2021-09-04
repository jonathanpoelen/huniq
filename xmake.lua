includes'cpp'

set_languages("cxx17")
set_config('ccache', false)

add_rules("mode.asan", "mode.debug", "mode.release", "mode.releasedbg")
set_allowedmodes('asan', 'debug', 'release', 'releasenative', 'releasedbg')
set_defaultmode('releasenative')

jln_cxx_init_options({warnings='very_strict', rtti='off'}, true)

jln_options = {}
if is_mode('debug') then
  jln_options.stl_debug = 'on'
elseif is_mode('asan') then
  -- set_optimize('none') -- by default xmake will optimize asan builds
  set_symbols('debug')
  jln_options.optimization = 'g'
  jln_options.stl_debug = 'on'
  jln_options.sanitizers = 'on'
elseif is_mode('release') then
  set_optimize('aggressive')
  jln_options.whole_program = 'strip_all'
elseif is_mode('releasedbg') then
  set_optimize('aggressive')
elseif is_mode('releasenative') then
  set_optimize('aggressive')
  -- jln_options.optimization = '3'
  jln_options.cpu = 'native'
  jln_options.lto = 'on'
  jln_options.whole_program = 'strip_all'
end

jln_cxx_rule('jln_flags', jln_options)
add_rules('jln_flags')

add_includedirs("include")

target('huniq')
  set_kind('binary')
  add_files('huniq.cpp')

target('test')
  add_deps('huniq')
  on_build(function(target)
    os.execv("tests/huniq.sh", {target:deps()['huniq']:targetfile()})
  end)
