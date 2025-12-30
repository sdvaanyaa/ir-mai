import os
import sys
import time
import random
import json
import yaml
import requests
import wikipediaapi
from bs4 import BeautifulSoup
from pathlib import Path
from urllib.parse import urljoin

class HistoryBot:
    def __init__(self, config_path):
        with open(config_path, 'r', encoding='utf-8') as f:
            self.cfg = yaml.safe_load(f)
        
        self.headers = {'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) MAI-Search-Project/2.0'}
        
        self.wiki = wikipediaapi.Wikipedia(
            user_agent='MAI-History-Student-Project/1.0',
            language='ru',
            extract_format=wikipediaapi.ExtractFormat.WIKI
        )
        
        self.raw_path = Path(self.cfg['raw_dir'])
        self.clean_path = Path(self.cfg['clean_dir'])
        self.raw_path.mkdir(parents=True, exist_ok=True)
        self.clean_path.mkdir(parents=True, exist_ok=True)
        
        self.progress = self._load_state()
        
        if not self.progress['wiki_queue']:
            self.progress['wiki_queue'] = [self.cfg['wiki_category']]

    def _load_state(self):
        if os.path.exists(self.cfg['state_file']):
            with open(self.cfg['state_file'], 'r', encoding='utf-8') as f:
                return json.load(f)
        return {
            "processed_urls": [], 
            "wiki_pages": [], 
            "wiki_queue": [], 
            "histrf_page": 1
        }

    def _save_state(self):
        with open(self.cfg['state_file'], 'w', encoding='utf-8') as f:
            json.dump(self.progress, f, ensure_ascii=False, indent=2)

    def crawl_wikipedia_step(self, batch_size=100):
        """Скачивает порцию статей из Википедии и запоминает, где остановился"""
        print(f"[*] Сбор порции из Википедии ({batch_size} шт.)...")
        saved_count = 0
        
        while self.progress['wiki_queue'] and saved_count < batch_size:
            current_cat_name = self.progress['wiki_queue'].pop(0)
            category = self.wiki.page(current_cat_name)
            
            try:
                members = category.categorymembers.values()
            except: continue

            for member in members:
                if saved_count >= batch_size: break
                
                if member.ns == wikipediaapi.Namespace.MAIN:
                    if member.title not in self.progress['wiki_pages']:
                        file_name = f"wiki_{member.pageid}.txt"
                        try:
                            with open(self.clean_path / file_name, 'w', encoding='utf-8') as f:
                                f.write(f"{member.title}\n\n{member.text}")
                            self.progress['wiki_pages'].append(member.title)
                            saved_count += 1
                        except: continue
                
                elif member.ns == wikipediaapi.Namespace.CATEGORY:
                    if member.title not in self.progress['wiki_queue']:
                        self.progress['wiki_queue'].append(member.title)
        
        print(f"    - Готово. Всего статей Википедии: {len(self.progress['wiki_pages'])}")
        self._save_state()

    def crawl_histrf_step(self, pages_to_scan=5):
        """Скачивает несколько страниц с История.рф"""
        print(f"[*] Сбор порции из История.рф ({pages_to_scan} стр.)...")
        
        for _ in range(pages_to_scan):
            p_num = self.progress['histrf_page']
            url = f"{self.cfg['histrf_url']}?page={p_num}"
            
            try:
                resp = requests.get(url, headers=self.headers, timeout=15)
                if resp.status_code != 200: break
                
                soup = BeautifulSoup(resp.text, 'lxml')
                links = [a['href'] for a in soup.find_all('a', href=True) if '/read/articles/' in a['href']]
                
                saved_on_page = 0
                for href in set(links):
                    full_url = urljoin("https://histrf.ru", href)
                    if full_url not in self.progress['processed_urls']:
                        if self._save_histrf_article(full_url):
                            self.progress['processed_urls'].append(full_url)
                            saved_on_page += 1
                
                print(f"    - Стр {p_num}: сохранено {saved_on_page}")
                self.progress['histrf_page'] += 1
                self._save_state()
                time.sleep(random.uniform(1, 2))
            except: break

    def _save_histrf_article(self, url):
        try:
            resp = requests.get(url, headers=self.headers, timeout=15)
            if resp.status_code != 200: return False
            
            slug = url.strip('/').split('/')[-1]
            with open(self.raw_path / f"hist_{slug}.html", 'w', encoding='utf-8') as f:
                f.write(resp.text)
            
            soup = BeautifulSoup(resp.text, 'lxml')
            title = soup.find('h1').get_text(strip=True) if soup.find('h1') else slug
            content = soup.select_one('.article-content') or soup.find('article')
            
            if content:
                for t in content.select('.popular, .related, script, style'): t.decompose()
                text = "\n".join(p.get_text(strip=True) for p in content.find_all('p') if len(p.get_text()) > 40)
                
                if len(text) > 500:
                    with open(self.clean_path / f"hist_{slug}.txt", 'w', encoding='utf-8') as f:
                        f.write(f"{title}\n\n{text}")
                    return True
            return False
        except: return False

if __name__ == "__main__":
    bot = HistoryBot("settings.yaml")
    
    print("=== Запуск добычи корпуса вперемешку ===")
    try:
        while True:
            bot.crawl_wikipedia_step(batch_size=100)
            
            bot.crawl_histrf_step(pages_to_scan=5)
            
            total = len(bot.progress['wiki_pages']) + len(bot.progress['processed_urls'])
            print(f"==> ВСЕГО В КОРПУСЕ: {total} документов\n")
            
            if total >= 35000: 
                print("Цель достигнута!")
                break
                
    except KeyboardInterrupt:
        print("\n[!] Остановлено. Состояние сохранено.")