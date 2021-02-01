# platformio path
pio = pio

# clang-format path
clang-format = clang-format-7

.PHONY: build
build:
	$(pio) run

.PHONY: upload
upload:
	$(pio) run -t upload

.PHONY: clean
clean:
	$(pio) run -t clean

.PHONY: monitor
monitor:
	$(pio) device monitor

.PHONY: format
format:
	find include lib src test \
		-type f \( -iname *.h -o -iname *.cpp -o -iname *.c \) \
	| xargs $(clang-format) -style=file -i

.PHONY: pre-push
pre-push: format
	$(pio) run $(foreach env,$(envs),-e $(env) )
