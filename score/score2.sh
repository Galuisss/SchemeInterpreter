#!/bin/bash
set -u  # 避免未定义变量

echo "Score script — show input, expected, actual"
echo "-------------------------------------------"

cd "$(dirname "$0")" || { echo "cd score dir failed"; exit 2; }

clean_output() {
  # $1: 输入文件，$2: 输出文件
  sed '$d' "$1" | sed 's/^scm> //' > "$2"
}

run_test() {
  local i="$1"
  local dir="$2"
  local in="$dir/$i.in"
  local out="$dir/$i.out"
  local raw="scm.raw.out"
  local cleaned="scm.out"

  echo ""
  echo "=========================== TEST $dir/$i ==========================="

  if [[ ! -f "$in" || ! -f "$out" ]]; then
    echo "Input or output file missing for $dir/$i, skip."
    return 2
  fi

  # 运行程序
  { cat "$in"; echo "(exit)"; } | ../build/code > "$raw"
  prog_rc=${PIPESTATUS[1]}   # 获取 ../build/code 的退出码

  clean_output "$raw" "$cleaned"

  echo "--- Input ($in) ---"
  cat "$in"
  echo "--- Expected ($out) ---"
  cat "$out"
  echo "--- Actual ($cleaned) ---"
  cat "$cleaned"
  echo "------------------------------"

  if diff -b "$cleaned" "$out" > diff_output.txt; then
    if [[ $prog_rc -ne 0 ]]; then
      echo "⚠️ PASS (output match) but program exit code $prog_rc"
    else
      echo "✅ PASS: $dir/$i"
    fi
    return 0
  else
    echo "❌ FAIL: $dir/$i"
    echo "Differences saved to diff_output.txt"
    return 1
  fi
}

summarize_range() {
  local dir="$1" L="$2" R="$3"
  local pass=0 fail=0 skip=0

  for ((i=L; i<=R; i++)); do
    run_test "$i" "$dir"
    rc=$?
    case $rc in
      0) ((pass++)) ;;
      1) ((fail++)) ;;
      2) ((skip++)) ;;
    esac
  done

  echo ""
  echo "===== SUMMARY for $dir [$L..$R] ====="
  echo "Passed: $pass"
  echo "Failed: $fail"
  echo "Skipped: $skip"
  echo "====================================="
}

# 主流程：测试 data/1..8
summarize_range "data" 1 60
