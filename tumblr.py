import pytumblr
import pprint
import time, datetime

client = pytumblr.TumblrRestClient('key goes here')

def is_gif(url):
    ext = url[-3:].lower()
    return ext == 'gif'

# Writes all photo urls in image posts from given blog to txt file
def get_blog_photos(blog, number_photos):

    filename = "%s-photos.txt" % blog
    f = open(filename, 'w')

    photos = 0
    while (photos < number_photos):
        print photos, number_photos

        response = client.posts(blog, type='photo', offset = photos)#, reblog_info=True)

        for post in response['posts']:
            for photo in post['photos']:
                photo_url = photo['original_size']['url'] + "\n"
                if is_gif(photo_url):
                    continue
                f.write(photo_url)
                photos += 1


    f.close()

# Writes all photo urls from given tag _above notes threshold_ to txt file
def get_tag_photos(tag, number_photos, notes_threshold):

    filename = "%s-tagged-photos.txt" % tag
    f = open(filename, 'w')

    d = datetime.date(2015, 8, 5)
    timestamp = int(time.mktime(d.timetuple()))

    photos = 0

    while photos < number_photos:
        print photos, number_photos
        if (photos > number_photos):
            return
        response = client.tagged(tag, before = timestamp)

        for post in response:
            if 'photos' not in post:
                continue

            notes = int(post['note_count'])

            for photo in post['photos']:
                photo_url = photo['original_size']['url'] + "\n"
                if notes >= notes_threshold and not is_gif(photo_url):
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
        client.blog_info(args[0])
        if 'meta' in client.blog_info(args[1]):
            if client.blog_info(args[1])['meta']['status'] == 404:
                print 'Blog not found'
                exit()
        get_blog_photos(args[1], int(args[2]))
    elif args[0] == 'tag':
        get_tag_photos(args[1], int(args[2]), int(args[3]))
    else:
        print 'Invalid option'

main()
