#!/usr/bin/env bash

find . -name "*:*" -exec rename 's|:|-|g' {} \;
