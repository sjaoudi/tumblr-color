import pytumblr
import pprint
import time

client = pytumblr.TumblrRestClient('API key goes here')
print 'Enter your API key in the script'
exit()

def is_gif(url):
    ext = url[-3:].lower()
    return ext == 'gif'


def get_blog_photos(blog, number_photos):

    filename = "%s-photos.txt" % blog
    f = open(filename, 'w')

    photos = 0
    while (photos < number_photos):

        response = client.posts(blog, type='photo', offset = 0)#, reblog_info=True)

        for post in response['posts']:
            for photo in post['photos']:
                photo_url = photo['original_size']['url'] + "\n"
                if is_gif(photo_url):
                    continue
                f.write(photo_url)
                photos += 1

    f.close()


def get_tag_photos(tag, number_photos, notes_threshold):

    filename = "%s-tagged-photos.txt" % tag
    f = open(filename, 'w')

    timestamp = int(time.time())

    photos = 0
    while (photos < number_photos):
        print photos
        response = client.tagged(tag, before = timestamp)

        for post in response:
            if 'photos' not in post:
                continue

            notes = post['note_count']

            for photo in post['photos']:
                photo_url = photo['original_size']['url'] + "\n"
                if notes >= notes_threshold:
                    f.write(photo_url)
                    photos += 1

            timestamp = post['timestamp']

    f.close()


def main():

    print "Usage: <blog|tag> <name of blog (full url) or tag> <photos to get> <notes threshold>"
    args = raw_input("> ").lower().split()

    if len(args) != 4:
        print 'Invalid option'
    elif args[0] == 'blog':
        if client.blog_info(args[1])['meta']['status'] == 404:
            print 'Blog not found'
            exit()
        get_blog_photos(args[1], args[2])
    elif args[0] == 'tag':
        get_tag_photos(args[1], args[2], args[3])
    else:
        print 'Invalid option'

main()
