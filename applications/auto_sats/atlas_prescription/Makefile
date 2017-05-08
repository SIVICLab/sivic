#
#   $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sat_placement_atlas_based/trunk/Makefile $
#   $Rev: 37691 $
#   $Author: jasonc@RADIOLOGY.UCSF.EDU $
#   $Date: 2016-02-23 13:01:30 -0800 (Tue, 23 Feb 2016) $
#
-include /netopt/share/include/include.mk



SCANNER_STARTUP_SCRIPT = svk_B1_map_analysis_startup
SCANNER_LOCAL_SCRIPT = svk_B1_map_analysis_local

PROG=Atlas_Based_Auto_MRSI_Prescription

SCRIPTS = 							\
	svk_atlas_auto_mrsi_start 		\
	svk_atlas_auto_mrsi_start.cfg   \
	svk_atlas_auto_mrsi_local 

FSLDIR = /netopt/rhel7/fsl
FSLDIR = /netopt/fsl5
THIRD_PARTY = /netopt/mricron/dcm2nii
FSL    = \
		${FSLDIR}/bin/fslval      			\
		${FSLDIR}/bin/fslstats				\
		${FSLDIR}/bin/remove_ext			\
		${FSLDIR}/bin/imtest				\
		${FSLDIR}/bin/bet2					\
		${FSLDIR}/bin/standard_space_roi	\
		${FSLDIR}/bin/flirt					\
		${FSLDIR}/bin/fslmaths				\
		${FSLDIR}/bin/fslroi				\
		${FSLDIR}/bin/fslmerge				\
		${FSLDIR}/bin/betsurf				\
		${FSLDIR}/bin/fast					\
		${FSLDIR}/bin/immv					\
		${FSLDIR}/bin/bet 


SCANNERS = cb-mr3tw
SCANNERS = qb3-mr7t 
SCANNERS = qb3-mr3t

SCANNER_BIN_DIR  = ~sdc/svk/console/packages/
SCANNER_LIB_DIR  = ~sdc/svk/console/lib/

HOST = scannersub

.PHONY: deploy

##########################
#   Scanner 
##########################
PKG_DIR     = $(PROG)
MAT_PKG_DIR = $(PKG_DIR)_matlab
ATLAS_DIR   = atlas_prescription

#
# Targets
#
ifneq (,$(filter $(ARCH)%,$(notdir $(CURDIR))))

############################################################
#
#   build, extract and configure startup shell script
#
############################################################
$(PROG):
	cd $(SRCDIR)/$(OBJDIR)/; pwd; compile_matlab $(ENV) $(MATLAB_INC) $(PREFIX) -D $(SRCDIR)/$(PROG); cd ../
	chmod -R 775 $(SRCDIR)/$(OBJDIR)/

endif

############################################################
#	this is the main target for building locally for testing 
############################################################
local: clean 
	rrc_deploy.pl -b build_local   
	#rrc_deploy.pl -m $(HOST) -b build_local   

build_local:
	$(MAKE) $(MFLAGS) OBJDIR="$(OBJDIR)" ENV=" -s " \
	MATLAB_INC="" PREFIX="" $(PROG)

############################################################
#	this is the main target for building and deploying onto 
#	the scanner
############################################################
deploy_scanner: clean 
	rrc_deploy.pl  -b build_scanner 
	rrc_deploy.pl -m $(HOST) -d install_scanner

build_scanner:
	$(MAKE) $(MFLAGS) OBJDIR="$(OBJDIR)_scanner" ENV="--env scanner_svk -s" \
	MATLAB_INC="$(MATLAB_SCANNER_INC)" PREFIX="" $(PROG)

install_scanner:
	mkdir -p ${PKG_DIR}/${MAT_PKG_DIR};
	mkdir -p ${PKG_DIR}/bin;
	cp -r ${PWD}/Linux_x86_64_scanner/* ${PKG_DIR}/${MAT_PKG_DIR}/; 
	cp -r ${PWD}/${ATLAS_DIR} ${PKG_DIR}/; 
	cp -r ~/svn/surbeck/brain/sat_placement_atlas_based/trunk/atlas_prescription/Atlas_default.nii ${PKG_DIR}/${ATLAS_DIR}
	for script in $(SCRIPTS) $(THIRD_PARTY); do	\
		cp $${script} ${PKG_DIR}/; 	\
	done; 
	for script in $(FSL); do	\
		cp $${script} ${PKG_DIR}/bin/; 	\
	done; 
	chmod -R 777 $(PKG_DIR) 
	for host in $(SCANNERS); do 																		\
		ssh -l sdc $$host "cd $(SCANNER_BIN_DIR); rm -rf $(SCANNER_BIN_DIR)/$(PKG_DIR)"; 				\
		scp -rp $(PKG_DIR) sdc@$$host:$(SCANNER_BIN_DIR)/; 												\
		ssh -l sdc $$host "cd $(SCANNER_LIB_DIR); rm -f ./matlab; ln -s /export/home/sdc/rrc/lib/matlab"; 	\
	done;


clean:
	${RM} -rf $(PKG_DIR) Linux*

-include /netopt/share/include/deps.mk

