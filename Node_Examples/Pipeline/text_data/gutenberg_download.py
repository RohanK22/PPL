import os
import requests

# Base URL for Project Gutenberg books
base_url = "https://www.gutenberg.org/cache/epub/{}/pg{}.txt"

# Function to download a book given its ID
def download_book(book_id):
    url = base_url.format(book_id, book_id)
    response = requests.get(url)
    if response.status_code == 200:
        # Save the book to a file
        filename = f"ebook_{book_id}.txt"
        with open(filename, 'wb') as file:
            file.write(response.content)
        print(f"Downloaded: {filename}")
    else:
        print(f"Failed to download book {book_id}")

# Number of books to download
num_books = 100

# IDs of books to download (you can choose any IDs you want)
book_ids = range(1, num_books + 1)

# Download each book
for book_id in book_ids:
    download_book(book_id)
