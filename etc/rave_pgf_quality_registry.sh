#!/usr/bin/env bash
set -x

grep -l ncarb_pid_quality_plugin ${RAVEROOT}/rave/etc/rave_pgf_quality_registry.xml
if [ $? -eq 1 ]
then
sed -i 's/<\/rave-pgf-quality-registry>/  <quality-plugin name="ncarb_pid" module="ncarb_pid_quality_plugin" class="ncarb_pid_quality_plugin"\/>\n<\/rave-pgf-quality-registry>/g' ${RAVEROOT}/rave/etc/rave_pgf_quality_registry.xml
fi
