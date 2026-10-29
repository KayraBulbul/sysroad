.PHONY: build install test run side

build:
	go build -o bin/sysroad .

install:
	go install .

test:
	go test ./...

run: build
	./bin/sysroad

side: build
	./bin/sysroad side
