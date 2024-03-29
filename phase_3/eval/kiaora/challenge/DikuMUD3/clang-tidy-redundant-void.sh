#!/bin/bash

#https://clang.llvm.org/extra/clang-tidy/checks/modernize-redundant-void-arg.html

tidy_binary=clang-tidy-12
which ${tidy_binary} > /dev/null 2>&1
if [ $? -ne 0 ]; then
   echo -e "\n\nERROR: clang-tidy needs to be installed"
   echo -e "\n    sudo apt-get install clang-tidy-12\n\n"
   if [ "$1" = "FEDORA" ]; then
      tidy_binary=clang-tidy
   else
      exit 1
   fi
fi

sources="
vme/src/defcomp/defcomp.cpp
vme/src/defcomp/defcomp.h
vme/src/defcomp/main.cpp
vme/src/mplex2/MUDConnector.h
vme/src/mplex2/echo_server.cpp
vme/src/mplex2/echo_server.h
vme/src/mplex2/main.cpp
vme/src/mplex2/mplex.cpp
vme/src/mplex2/mplex.h
vme/src/mplex2/network.cpp
vme/src/mplex2/network.h
vme/src/mplex2/telnet.h
vme/src/mplex2/translate.cpp
vme/src/mplex2/translate.h
vme/src/mplex2/ttydef.h
vme/src/mplex2/ClientConnector.cpp
vme/src/mplex2/ClientConnector.h
vme/src/mplex2/MUDConnector.cpp
vme/src/vmc/dilpar.h
vme/src/vmc/main.cpp
vme/src/vmc/vmc.h
vme/src/vmc/vmc_process.h
vme/src/vmc/vmc.cpp
vme/src/vmc/vmc_process.cpp
vme/src/account.h
vme/src/act_change.h
vme/src/act_color.h
vme/src/act_info.h
vme/src/act_movement.cpp
vme/src/act_movement.h
vme/src/act_offensive.cpp
vme/src/act_offensive.h
vme/src/act_other.h
vme/src/act_skills.cpp
vme/src/act_skills.h
vme/src/act_wizard.h
vme/src/affect.cpp
vme/src/affect.h
vme/src/affectdil.h
vme/src/apf_affect.cpp
vme/src/apf_affect.h
vme/src/badnames.cpp
vme/src/badnames.h
vme/src/ban.h
vme/src/bank.h
vme/src/bytestring.cpp
vme/src/bytestring.h
vme/src/cmdload.h
vme/src/combat.h
vme/src/common.cpp
vme/src/compile_defines.h
vme/src/constants.cpp
vme/src/constants.h
vme/src/convert.h
vme/src/db.h
vme/src/db_file.h
vme/src/dbfind.cpp
vme/src/dbfind.h
vme/src/destruct.cpp
vme/src/destruct.h
vme/src/dictionary.h
vme/src/diku_exception.h
vme/src/dil.h
vme/src/dilexp.h
vme/src/dilinst.cpp
vme/src/dilinst.h
vme/src/dilrun.cpp
vme/src/dilrun.h
vme/src/dilshare.cpp
vme/src/dilshare.h
vme/src/dilsup.cpp
vme/src/dilsup.h
vme/src/eliza.h
vme/src/event.h
vme/src/experience.h
vme/src/extra.cpp
vme/src/extra.h
vme/src/fight.h
vme/src/files.cpp
vme/src/files.h
vme/src/guild.h
vme/src/handler.cpp
vme/src/handler.h
vme/src/hook.cpp
vme/src/hook.h
vme/src/hookmud.cpp
vme/src/hookmud.h
vme/src/interpreter.cpp
vme/src/interpreter.h
vme/src/intlist.cpp
vme/src/intlist.h
vme/src/justice.cpp
vme/src/justice.h
vme/src/magic.cpp
vme/src/magic.h
vme/src/main.cpp
vme/src/main.h
vme/src/membug.cpp
vme/src/membug.h
vme/src/mobact.cpp
vme/src/mobact.h
vme/src/modify.h
vme/src/movement.h
vme/src/namelist.cpp
vme/src/namelist.h
vme/src/nanny.h
vme/src/nice.h
vme/src/path.cpp
vme/src/path.h
vme/src/pipe.h
vme/src/protocol.cpp
vme/src/protocol.h
vme/src/queue.cpp
vme/src/queue.h
vme/src/sector.cpp
vme/src/sector.h
vme/src/signals.cpp
vme/src/signals.h
vme/src/skills.h
vme/src/slime.h
vme/src/spec_assign.cpp
vme/src/spec_assign.h
vme/src/spec_procs.cpp
vme/src/spec_procs.h
vme/src/spell_parser.h
vme/src/spells.h
vme/src/str_parse.cpp
vme/src/str_parse.h
vme/src/structs.h
vme/src/system.h
vme/src/t_array.h
vme/src/teach.h
vme/src/tif_affect.cpp
vme/src/tif_affect.h
vme/src/trie.cpp
vme/src/trie.h
vme/src/unitfind.cpp
vme/src/unitfind.h
vme/src/utility.cpp
vme/src/utils.h
vme/src/vmelimits.h
vme/src/weather.h
vme/src/zon_wiz.h
vme/src/zone_reset.cpp
vme/src/zone_reset.h
vme/src/bank.cpp
vme/src/money.h
vme/src/OutputCapture.cpp
vme/src/OutputCapture.h
vme/src/account.cpp
vme/src/act_change.cpp
vme/src/act_color.cpp
vme/src/act_info.cpp
vme/src/act_other.cpp
vme/src/act_wizard.cpp
vme/src/act_wstat.cpp
vme/src/act_wstat.h
vme/src/ban.cpp
vme/src/cmdload.cpp
vme/src/color.cpp
vme/src/color.h
vme/src/combat.cpp
vme/src/comm.cpp
vme/src/comm.h
vme/src/common.h
vme/src/config.cpp
vme/src/config.h
vme/src/convert.cpp
vme/src/db.cpp
vme/src/db_file.cpp
vme/src/dictionary.cpp
vme/src/dilexp.cpp
vme/src/dilfld.cpp
vme/src/eliza.cpp
vme/src/error.h
vme/src/essential.h
vme/src/event.cpp
vme/src/experience.cpp
vme/src/fight.cpp
vme/src/formatter.h
vme/src/guild.cpp
vme/src/main_functions.cpp
vme/src/main_functions.h
vme/src/modify.cpp
vme/src/money.cpp
vme/src/nanny.cpp
vme/src/nice.cpp
vme/src/pcsave.cpp
vme/src/pcsave.h
vme/src/pipe.cpp
vme/src/reception.cpp
vme/src/reception.h
vme/src/skills.cpp
vme/src/slime.cpp
vme/src/slog.h
vme/src/slog_raw.cpp
vme/src/slog_raw.h
vme/src/spell_parser.cpp
vme/src/structs.cpp
vme/src/system.cpp
vme/src/szonelog.h
vme/src/szonelog_raw.cpp
vme/src/szonelog_raw.h
vme/src/teach.cpp
vme/src/textutil.cpp
vme/src/textutil.h
vme/src/utility.h
vme/src/vmelimits.cpp
vme/src/weather.cpp
vme/src/zon_basis.cpp
vme/src/zon_basis.h
vme/src/zon_wiz.cpp
"

# Create clean fresh compile_commands.json file for clang-tidy
tmpdir=$(mktemp -d -p .)
pushd $tmpdir
cmake ..
popd

for source in $sources
do
   echo "Selecting [$source] for tidying!"
   ${tidy_binary} -p $tmpdir --quiet --checks="-*,modernize-redundant-void-arg" --fix $source || exit 1
   clang-format -i $source
done
rm -rf $tmpdir
echo -e "\n\nFINISHED!\n\n"
