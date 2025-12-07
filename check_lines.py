import os

def count_lines_and_sort(target_path, extensions=None, exclude_dirs=None, top_n=None):
    """
    폴더 내 파일의 라인 수를 세고, 라인 수가 많은 순서대로 정렬하여 출력합니다.
    
    :param top_n: 상위 몇 개의 파일만 출력할지 설정 (None이면 전체 출력)
    """
    if exclude_dirs is None:
        exclude_dirs = ['.git', '__pycache__', 'venv', 'node_modules', '.idea', '.vscode']
    
    # 결과를 저장할 리스트 (라인수, 파일경로)
    results = []
    
    total_lines = 0
    total_files = 0
    
    print(f"--- '{target_path}' 분석 중... ---")

    for root, dirs, files in os.walk(target_path):
        # 제외 폴더 필터링
        dirs[:] = [d for d in dirs if d not in exclude_dirs]
        
        for file in files:
            if extensions:
                if not any(file.endswith(ext) for ext in extensions):
                    continue
            
            full_path = os.path.join(root, file)
            
            # 보기 좋게 상대 경로로 변환 (선택 사항)
            rel_path = os.path.relpath(full_path, target_path)

            try:
                # 1. UTF-8 시도
                with open(full_path, 'r', encoding='utf-8') as f:
                    line_count = sum(1 for _ in f)
            except UnicodeDecodeError:
                try:
                    # 2. CP949 시도
                    with open(full_path, 'r', encoding='cp949') as f:
                        line_count = sum(1 for _ in f)
                except Exception:
                    continue # 읽기 실패시 건너뜀

            # 결과 리스트에 (라인수, 경로) 튜플로 추가
            results.append((line_count, rel_path))
            total_lines += line_count
            total_files += 1

    # --- 정렬 로직 (핵심) ---
    # 튜플의 첫 번째 요소(라인수)를 기준으로 내림차순(reverse=True) 정렬
    results.sort(key=lambda x: x[0], reverse=True)

    # --- 결과 출력 ---
    print("\n" + "="*60)
    print(f" {'LINE COUNT':>10} | FILE PATH")
    print("="*60)
    
    # 상위 N개 또는 전체 출력
    limit = top_n if top_n else len(results)
    
    for count, path in results[:limit]:
        print(f" {count:>10,} | {path}")

    print("-" * 60)
    print(f" [SUMMARY]")
    print(f"  * Total Files : {total_files}")
    print(f"  * Total Lines : {total_lines:,}")
    print("=" * 60)

# --- 실행 설정 ---
if __name__ == "__main__":
    target_folder = "./src"  # 현재 폴더
    target_extensions = ['.py', '.java', '.js', '.html', '.css', '.c', '.cpp']
    
    # top_n=20 : 상위 20개만 출력 (전체를 보려면 None 입력)
    count_lines_and_sort(target_folder, extensions=target_extensions, top_n=20)