DEPS = line_in_select_channel
OBJ = amAudio.o line_in_select_channel.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

amaudio: $(OBJ)
	$(CXX) -lasound -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ amaudio

.PHONY: install

install:
	cp -f amaudio $(TARGET_DIR)/usr/bin/

.PHONY: uninstall

uninstall:
	rm -f $(TARGET_DIR)/usr/bin/amaudio
