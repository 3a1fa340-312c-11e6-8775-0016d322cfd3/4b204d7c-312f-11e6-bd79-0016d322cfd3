#                Copyright 2006, ZOT.

# Rule to create $(OBJ_DIR) if $(OBJ_DIR) does not exist
MAKE_OBJ_DIR:
	@-if !(test -d $(OBJ_DIR)); then mkdir $(OBJ_DIR); fi;

# Rule to create $(LIB_DIR) if $(LIB_DIR) does not exist
MAKE_LIB_DIR:
	@-if !(test -d $(LIB_DIR)); then mkdir $(LIB_DIR); fi;

RM_AXF_FILE:
	@-if (test -f $(PROD_NAME).axf); then rm $(PROD_NAME).axf; fi;

clean:
	-rm -f $(OBJS)
	-rm -f ${DST}
	-rm -f $(DEPEND_FILES)
	-rm -f .depend

# Dependencies
DEPEND_FILES = ${SRCS:%.c=%.d}

depend: $(DEPEND_FILES)
	/bin/find .. -name "*.d" | xargs cat >.depend


-include .depend

