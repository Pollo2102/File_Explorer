#include "FElement.h"
#include "FWindow.h"
#include <vector>
#include <cstring>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <filesystem>
#include <experimental/filesystem>
#include <cmath>

#define INITIAL_FILE_X_POSITION 20
#define INITIAL_FILE_Y_POSITION 70
#define FILE_BITMAP "../img/file.xbm"
#define FOLDER_BITMAP "../img/folder.xbm"
#define LINK_BITMAP "../img/big-link.xbm"

std::vector<XRectangle> files;
std::vector<BitmapData> file_icons;
std::vector<XRectangle> ui_elems;
std::vector<BitmapData> bitmaps;
std::vector<FileData> files_info;

uint32_t file_X_pos = INITIAL_FILE_X_POSITION;
uint32_t file_Y_pos = INITIAL_FILE_Y_POSITION;

uint32_t page_number = 0;

bool deleteFlag = 0;
bool copyFlag = 0; 
bool moveFlag = 0; 
bool openFlag = 0; 
bool symlinkFlag = 0;
bool symlinkBuffer = 0;
bool isFile = 0;
bool isFolder = 0;
bool moveFileBuffer = 0;

std::string WINDOW_TITLE = "Sistemas Operativos - File Explorer";
std::string current_path = "/";
const std::string subwindow_subtext = "Write your filename here:";
std::string subtext = "";

std::string copyFilePath = "";
std::string copyFileName = "";
std::string moveFilePath = "";
std::string moveFileName = "";
std::string symlinkFilePath = "";
std::string symlinkFileName = "";

FWindow::FWindow()
{
    init_server();
    init_window();
    open_window();
    get_current_dir_files();
}

void FWindow::init_server()
{
    d = XOpenDisplay(nullptr);
    if (d == nullptr)
    {
        printf("Cannot open display\n");
        exit(1);
    }
    s = DefaultScreen(d);
}

/* open connection with the server */
void FWindow::init_window()
{
    /* create window */
    w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 800, 600, 1,
                            BlackPixel(d, s), WhitePixel(d, s));

    // Process Window Close Event through event handler so XNextEvent does Not fail
    Atom delWindow = XInternAtom(d, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(d, w, &delWindow, 1);

    /* select kind of events we are interested in */
    XSelectInput(d, w, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | ExposureMask);

    set_window_title(&WINDOW_TITLE[0]);
    ui_elems.push_back(create_rectangle(0, 45, 800, 5));

    bitmaps.push_back(createBitmap("../img/open.xbm"));
    bitmaps.push_back(createBitmap("../img/up-arrow.xbm"));
    bitmaps.push_back(createBitmap("../img/trash.xbm"));
    bitmaps.push_back(createBitmap("../img/create_folder.xbm"));
    bitmaps.push_back(createBitmap("../img/create_file.xbm"));
    bitmaps.push_back(createBitmap("../img/copy.xbm"));
    bitmaps.push_back(createBitmap("../img/move.xbm"));
    bitmaps.push_back(createBitmap("../img/link.xbm"));
    bitmaps.push_back(createBitmap("../img/up-move.xbm"));
    bitmaps.push_back(createBitmap("../img/down-move.xbm"));

    /* map (show) the window */
    XMapWindow(d, w);
}

void FWindow::open_window()
{
    get_current_dir_files();
    while (1)
    {
        XNextEvent(d, &e);
        /* draw or redraw the window */
        if (e.type == Expose)
        {
            print_screen();
        }
        /* exit on key press */
        if (e.type == KeyPress)
            break;
        
        if (e.type == ButtonPress)
        {
            if (e.xbutton.button == Button1)
                handle_mouse_click();
        }

        // Handle Windows Close Event
        if (e.type == ClientMessage)
            break;
    }

    /* destroy our window */
    XDestroyWindow(d, w);

    /* close connection to server */
    XCloseDisplay(d);
}

void FWindow::print_screen()
{
    file_X_pos = INITIAL_FILE_X_POSITION;
    file_Y_pos = INITIAL_FILE_Y_POSITION;
    BitmapData BMD;
    // Draw elements on screen
    XDrawString(d, w, DefaultGC(d, s), 420, 28, &current_path[0], current_path.size());
    XFillRectangles(d, w, DefaultGC(d, s), &ui_elems[0], ui_elems.size());
    for (size_t i = 0; i < 25; i++)
    {
        if (!((page_number * 25) + i < files_info.size()))
        {
            break;
        }
        auto &finfo = files_info[(page_number * 25) + i];

        if (finfo.file_type == DT_DIR)
        {
            BMD = createBitmap(FOLDER_BITMAP);
            XCopyPlane(d, BMD.P, w, DefaultGC(d, s), 0, 0, BMD.width, BMD.height, file_X_pos, file_Y_pos, 1);
            XDrawString(d, w, DefaultGC(d, s), file_X_pos + 20, file_Y_pos + 97, &finfo.filename[0], finfo.filename.size());
        }
        else if (finfo.file_type == DT_LNK)
        {
            BMD = createBitmap(LINK_BITMAP);
            XCopyPlane(d, BMD.P, w, DefaultGC(d, s), 0, 0, BMD.width, BMD.height, file_X_pos, file_Y_pos, 1);
            XDrawString(d, w, DefaultGC(d, s), file_X_pos + 20, file_Y_pos + 110, &finfo.filename[0], finfo.filename.size());
        }
        else/*  if (finfo.file_type == DT_REG) */
        {
            BMD = createBitmap(FILE_BITMAP);
            XCopyPlane(d, BMD.P, w, DefaultGC(d, s), 0, 0, BMD.width, BMD.height, file_X_pos, file_Y_pos, 1);
            XDrawString(d, w, DefaultGC(d, s), file_X_pos + 20, file_Y_pos + 110, &finfo.filename[0], finfo.filename.size());
        }
        finfo.height = BMD.height;
        finfo.width = BMD.width;
        finfo.x = file_X_pos;
        finfo.y = file_Y_pos;
        if (file_X_pos == 620)
        {
            file_X_pos = 20;
            file_Y_pos += 110;
        }
        else 
            file_X_pos += 150;
    }
    
    // for(auto &finfo : files_info)
    // {
    //     if (finfo.file_type == DT_DIR)
    //     {
    //         BMD = createBitmap(FOLDER_BITMAP);
    //         XCopyPlane(d, BMD.P, w, DefaultGC(d, s), 0, 0, BMD.width, BMD.height, file_X_pos, file_Y_pos, 1);
    //         XDrawString(d, w, DefaultGC(d, s), file_X_pos + 20, file_Y_pos + 97, &finfo.filename[0], finfo.filename.size());
    //     }
    //     else/*  if (finfo.file_type == DT_REG) */
    //     {
    //         BMD = createBitmap(FILE_BITMAP);
    //         XCopyPlane(d, BMD.P, w, DefaultGC(d, s), 0, 0, BMD.width, BMD.height, file_X_pos, file_Y_pos, 1);
    //         XDrawString(d, w, DefaultGC(d, s), file_X_pos + 20, file_Y_pos + 110, &finfo.filename[0], finfo.filename.size());
    //     }
    //     finfo.height = BMD.height;
    //     finfo.width = BMD.width;
    //     finfo.x = file_X_pos;
    //     finfo.y = file_Y_pos;
    //     if (file_X_pos == 620)
    //     {
    //         file_X_pos = 20;
    //         file_Y_pos += 110;
    //     }
    //     else 
    //         file_X_pos += 150;
    // }
    print_icons();
}

void FWindow::handle_mouse_click()
{
    int x_root, y_root, x_child, y_child;
    Window root;
    Window child;
    unsigned int mask;
    if (XQueryPointer(d, w, &root, &child, &x_root, &y_root, &x_child, &y_child, &mask))
    {
        check_mouse_coordinates(x_child, y_child);
    }
}

void FWindow::check_mouse_coordinates(uint32_t x_coor, uint32_t y_coor)
{
    for(auto a : bitmaps)
    {
        if (x_coor >= a.x 
            && x_coor <= (a.x + a.width)
            && y_coor >= a.y
            && y_coor <= (a.y + a.height))
        {
            // Handle icon press
            if (a.filename == "../img/open.xbm")
            {
                openFlag = 1;
            }
            else if(a.filename == "../img/up-arrow.xbm")
            {
                change_dir("..");
                get_current_dir_files();
                print_screen();
            }
            else if(a.filename == "../img/trash.xbm")
            {
                deleteFlag = 1;
            }
            else if(a.filename == "../img/create_folder.xbm")
            {
                init_subwindow();
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                {
                    if (chdir(current_path.c_str()) != -1)
                    {
                        if (mkdir(subtext.c_str(), 0744) == -1)
                            std::cout << "Error creating directory.\n";
                        else
                            std::cout << "Directory created successfully\n";
                    }
                    if (chdir(cwd) == -1)
                    {
                        std::cout << "Error returning to project directory.\n";
                        std::exit(1);   
                    }
                }
            }
            else if(a.filename == "../img/create_file.xbm")
            {
                init_subwindow();
                if (open((current_path + subtext).c_str(), O_CREAT, O_RDWR) != -1)
                    std::cout << "Created file" << std::endl;
                subtext.clear();
            }
            else if(a.filename == "../img/copy.xbm")
            {
                std::cout << "Copying file\n";
                std::cout << "Filename: " << copyFilePath << std::endl;
                std::cout << "isFlag: " << isFile << std::endl;
                if (!copyFilePath.empty() && isFile)
                {
                    std::cout << "Copying file now\n";
                    // Copy file in buffer to current dir
                    int input, output;    
                    if ((input = open(copyFilePath.c_str(), O_RDONLY)) == -1)
                    {
                        std::cout << "Input error\n";
                        return;
                    }    
                    if ((output = creat((current_path + copyFileName).c_str(), 0660)) == -1)
                    {
                        std::cout << "Output error\n";
                        close(input);
                        return;
                    }
                    off_t bytesCopied = 0;
                    struct stat fileInfo = {0};
                    fstat(input, &fileInfo);
                    sendfile(output, input, &bytesCopied, fileInfo.st_size);
                    close(input);
                    close(output);
                    copyFilePath.empty();
                }
                else if (!copyFilePath.empty() && isFolder)
                {
                    // Copy folder
                    std::filesystem::path source = copyFilePath;
                    std::filesystem::path destination = current_path + copyFileName;
                    std::filesystem::copy(source, destination, std::filesystem::copy_options::recursive);
                    copyFilePath.clear();
                }
                std::cout << "Copying file2\n";
                if (copyFilePath.empty())
                    copyFlag = 1;
            }
            else if(a.filename == "../img/move.xbm")
            {
                if (moveFileBuffer)
                {
                    if (!moveFilePath.empty())
                    {
                        if (rename (moveFilePath.c_str(), (current_path + moveFileName).c_str())) {
                            // something went wrong
                            if (errno == EXDEV) {
                                // copy data and meta data 
                            } 
                            else { perror("rename"); exit(EXIT_FAILURE); };
                        } 
                        else 
                        {
                             std::cout << "Move succeeded\n";
                             moveFileBuffer = 0;
                             moveFilePath.empty();
                             moveFileName.empty();
                        }
                    }
                }
                else 
                {
                    moveFlag = 1;
                }
            }
            else if (a.filename == "../img/link.xbm")
            {
                if (!symlinkBuffer)
                    symlinkFlag = 1;
                else 
                {
                    if (!symlinkFilePath.empty())
                    {
                        int res = symlink(symlinkFilePath.c_str(), (current_path + symlinkFileName).c_str());
                        if (!res)
                            printf("Link created.\n");
                        else
                            printf("Link creation failed.\n");
                    }
                    symlinkBuffer = 0;
                }
            }
            else if (a.filename == "../img/down-move.xbm")
            {
                if (page_number < (uint32_t)std::ceil(files_info.size() % 25))
                {
                    page_number++;
                }
            }
            else if (a.filename == "../img/up-move.xbm")
            {
                if (page_number > 0)
                    page_number--;
            }
            XClearWindow(d, w);
            get_current_dir_files();
            print_screen();
        }
    }

    for(auto a : files_info)
    {
        if (x_coor >= a.x 
            && x_coor <= (a.x + a.width)
            && y_coor >= a.y
            && y_coor <= (a.y + a.height))
        {
            // Handle icon press
            // std::cout << a.filename << std::endl;
            if (deleteFlag)
            {
                if (a.file_type == DT_DIR)
                {
                    // rmdir((current_path + a.filename).c_str());
                    std::experimental::filesystem::remove_all(current_path + a.filename);
                }
                else if (a.file_type == DT_REG || a.file_type == DT_LNK)
                    remove((current_path + a.filename).c_str());
                    
                deleteFlag = 0;
                XClearWindow(d, w);
                get_current_dir_files();
                print_screen();
            }
            else if (copyFlag)
            {
                if (a.file_type == DT_DIR)
                {
                    isFolder = 1;
                    isFile = 0;
                }
                else if (a.file_type == DT_REG)
                {
                    isFile = 1;
                    isFolder = 0;
                }
                copyFilePath = a.file_abs_path;
                copyFileName = a.filename;

                copyFlag = 0;
                
            }
            else if (moveFlag)
            {
                if (a.file_type == DT_DIR)
                {
                    isFolder = 1;
                    isFile = 0;
                }
                else if (a.file_type == DT_REG)
                {
                    isFile = 1;
                    isFolder = 0;
                }
                moveFileName = a.filename;
                moveFilePath = a.file_abs_path;
                
                moveFlag = 0;
                moveFileBuffer = 1;
            }
            else if (openFlag)
            {
                if (a.file_type == DT_REG)
                {
                    std::string s;
                    s = ("xdg-open " + a.file_abs_path);
                    int res = system(s.c_str());
                    if (res)
                        printf("Error opening %s\n", a.filename.c_str());
                }

                openFlag = 0;
            }
            else if (symlinkFlag)
            {
                symlinkFileName = a.filename;
                symlinkFilePath = a.file_abs_path;

                symlinkFlag = 0;
                symlinkBuffer = 1;
            }
            else if (a.file_type == DT_DIR)
            {
                page_number = 0;
                change_dir(a.filename);
                print_screen();
            }
        }
    }
}

void FWindow::set_window_title(char * title)
{
    XStoreName(d, w, title);
}

XRectangle FWindow::create_rectangle(int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    XRectangle XR;
    XR.x = x;
    XR.y = y;
    XR.width = width;
    XR.height = height;
    return XR;
}


BitmapData FWindow::createBitmap(std::string filename)
{
    /* this variable will contain the ID of the newly created pixmap.    */
    Pixmap bitmap;
    BitmapData BD;

    /* these variables will contain the dimensions of the loaded bitmap. */
    unsigned int bitmap_width, bitmap_height;

    /* these variables will contain the location of the hot-spot of the   */
    /* loaded bitmap.                                                    */
    int hotspot_x, hotspot_y;

    /* this variable will contain the ID of the root window of the screen */
    /* for which we want the pixmap to be created.                        */
    Window root_win = DefaultRootWindow(d);

    /* load the bitmap found in the file "icon.bmp", create a pixmap      */
    /* containing its data in the server, and put its ID in the 'bitmap'  */
    /* variable.                                                          */
    int rc = XReadBitmapFile(d, root_win,
                            filename.c_str(),
                            &bitmap_width, &bitmap_height,
                            &bitmap,
                            &hotspot_x, &hotspot_y);
    /* check for failure or success. */
    switch (rc) {
        case BitmapOpenFailed:
            fprintf(stderr, "XReadBitmapFile - could not open file %s.\n", &filename[0]);
            break;
        case BitmapFileInvalid:
            fprintf(stderr,
                    "XReadBitmapFile - file '%s' doesn't contain a valid bitmap.\n",
                    &filename[0]);
            break;
        case BitmapNoMemory:
            fprintf(stderr, "XReadBitmapFile - not enough memory.\n");
            break;
        case BitmapSuccess:
            /* bitmap loaded successfully - do something with it... */
            BD.P = bitmap;
            BD.height = bitmap_height;
            BD.width = bitmap_width;
            BD.filename = filename;
            return BD;
            break;
    }
    return BD;
}

void FWindow::get_current_dir_files()
{
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(current_path.c_str())) != NULL)
        {
            /* print all the files and directories within directory */
            files_info.clear();
            while ((ent = readdir(dir)) != NULL)
            {
                if (strcmp(ent->d_name, "."))
                {
                    FileData FD;
                    FD.filename = ent->d_name;
                    FD.file_abs_path = current_path + ent->d_name;
                    FD.file_type = ent->d_type;
                    // printf("%s\n", ent->d_name);
                    files_info.push_back(FD);
                }
            }
            closedir(dir);
        }
        else
        {
            /* could not open directory */
            perror("Couldn't open the directory.\n");
            return;
        }
}

void FWindow::open_file(dirent *entry)
{
    if (entry->d_type == DT_UNKNOWN)
        std::cout << "Unkown file type.\n";
    else if(entry->d_type == DT_REG)
        // Open the file
        std::exit(1);
    else if(entry->d_type == DT_DIR)
        change_dir(entry->d_name);
}

void FWindow::change_dir(std::string dir_string)
{
    if (dir_string == "..")
    {
        if (current_path != "/")
        {
            current_path.pop_back();
            while (current_path[current_path.size() - 1] != '/')
            {
                current_path.pop_back();
            }
            XClearWindow(d, w);
            get_current_dir_files();
        }
    }
    else
    {
        current_path += (dir_string + "/");
        XClearWindow(d, w);
        get_current_dir_files();

    }
}

void FWindow::print_icons()
{
    uint32_t initial_x_icons = 15;
    for(auto &a : bitmaps)
    {
        XCopyPlane(d, a.P, w, DefaultGC(d, s), 0, 0, a.width, a.height, initial_x_icons, 5, 1);
        a.x = initial_x_icons;
        a.y = 5;
        initial_x_icons += 40;
    }
}

XTextItem makeXTextItem(std::string text, uint32_t X, uint32_t Y)
{
    XTextItem XTxt;
    XTxt.chars = &text[0];
    XTxt.nchars = text.size();
    XTxt.delta = 1;
    return XTxt;
}

void FWindow::init_subwindow()
{
    /* create window */

    Window w2 = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 400, 200, 1,
                            BlackPixel(d, s), WhitePixel(d, s));

    // Process Window Close Event through event handler so XNextEvent does Not fail
    Atom delWindow = XInternAtom(d, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(d, w2, &delWindow, 1);

    /* select kind of events we are interested in */
    XSelectInput(d, w2, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | ExposureMask);

    // set_window_title(&WINDOW_TITLE[0]);
    XStoreName(d, w2, "Type a string");

    /* map (show) the window */
    XMapWindow(d, w2);

    while (1)
    {
        XNextEvent(d, &e);
        /* draw or redraw the window */
        if (e.type == Expose)
        {
            XDrawString(d, w2, DefaultGC(d, s), 120, 65, &subwindow_subtext[0], subwindow_subtext.size());
            XDrawString(d, w2, DefaultGC(d, s), 120, 105, &subtext[0], subtext.size());
        }
        /* exit on key press */
        if (e.type == KeyPress)
        {
            char tmpChar[2];
            KeySym keysym_return;
            int len = XLookupString(&e.xkey, tmpChar, 1, &keysym_return, nullptr);
            if (tmpChar[0] == 13)
            {
                // Create the file or directory
                break;
            }
            else if (!len) continue;
            else if (tmpChar[0] == 8 && !subtext.empty()) subtext.pop_back();
            else if (tmpChar[0] != 8) subtext += tmpChar[0];    

            XClearWindow(d, w2);
            XDrawString(d, w2, DefaultGC(d, s), 120, 65, &subwindow_subtext[0], subwindow_subtext.size());
            XDrawString(d, w2, DefaultGC(d, s), 120, 105, &subtext[0], subtext.size());
        }
        
        if (e.type == ButtonPress)
        {
            if (e.xbutton.button == Button1)
                handle_mouse_click();
        }

        // Handle Windows Close Event
        if (e.type == ClientMessage)
            break;
    }

    /* destroy our window */
    XDestroyWindow(d, w2);
}