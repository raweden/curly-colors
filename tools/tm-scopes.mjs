//#!/usr/bin/env -S node --experimental-default-type=module

// simple command-line utility that generates a sequence of macros for pre-defining
// scopes used in a scope repository.

import { accessSync, readFileSync } from "node:fs";

const USAGE_TXT = `usage:
    tm-scopes <filepath>`;

function processFile(filepath) {
    let data = readFileSync(filepath, "utf8");
    data = data.trim();
    let newLines, lines = data.split('\n');
    let ylen, xlen = lines.length;
    newLines = [];
    for (let i = 0; i < xlen; i++) {
        let ln = lines[i];
        ln = ln.replace(/\#[^\n]*$/g, ''); // basic support for # comments.
        ln = ln.trim();
        if (ln.length != 0) {
            newLines.push(ln);
        }
    }

    lines = newLines;
    let scopes = [];
    let depth = 0;
    xlen = lines.length;

    for (let i = 0; i < xlen; i++) {
        let str = lines[i];
        let arr = str.split('.');
        lines[i] = arr;
        ylen = arr.length;
        if (ylen > depth) {
            depth = ylen;
        }
    }

    ylen = depth;
    for (let y = 0; y < ylen; y++) {

        for (let x = 0; x < xlen; x++) {
            let arr = lines[x];
            if (y >= arr.length) {
                continue;
            }
            let str = arr[y];
            if (scopes.indexOf(str) == -1) {
                scopes.push(str);
            }
        }
    }

    xlen = scopes.length;
    for (let i = 0; i < xlen; i++) {
        let str = scopes[i];
        console.log("DECLARE_STD_SCOPE(\"" + str + "\"),");
    }
}

function printHelp() {
    console.log(USAGE_TXT);
    process.exit(0);
}

function main() {

    let arg2;
    let args = process.argv;
    let validFile = false;

    if (args.length > 2) {
        arg2 = args[2];
    }

    if (arg2 == '--help') {
        printHelp();
    }

    if (arg2 === undefined) {
        arg2 = "scopes.txt";
    }

    try {
        accessSync(arg2);
        validFile = true;
    } catch (err) {
        console.log("could not access '%s' error = %s", arg2, err.code);
        process.exit(-1);
    }

    processFile(arg2);
}

main();

