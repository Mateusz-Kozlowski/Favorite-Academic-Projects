import matplotlib.pyplot as plt
import matplotlib.image as mpimg

import pandas as pd
import numpy as np

from analyzer import const


def visualize_speed_dist(df, speed_dist_png_path):
    speeds = pd.to_numeric(df['Speed'], errors='coerce').dropna()

    plt.figure(figsize=(10, 6))
    plt.hist(
        speeds,
        bins=np.arange(1, const.MAX_SPEED_ON_GRAPH, 5),
        color='blue',
        edgecolor='black',
        alpha=0.7
    )
    plt.title('Distribution of buses speeds')
    plt.xlabel('Speed (km/h)')
    plt.ylabel('Number of buses speed measurements')
    # Set the x-axis limit to the max_speed value:
    plt.xlim(0, const.MAX_SPEED_ON_GRAPH)
    plt.grid(True)
    plt.savefig(speed_dist_png_path)
    print('Speed distribution saved to', speed_dist_png_path)
    plt.show()


def remove_axes_tick_marks(ax):
    ax.set_xticks([])
    ax.set_yticks([])


def draw_grid(cols_cnt, rows_cnt, ax, grid_width_on_img, grid_height_on_img):
    # Draw vertical grid lines
    for i in range(cols_cnt - 1):
        line_lon = (i + 1) * grid_width_on_img
        ax.axvline(x=line_lon, color='grey', linestyle='--', linewidth=1)

    # Draw horizontal grid lines
    for j in range(rows_cnt - 1):
        line_lat = (j + 1) * grid_height_on_img
        ax.axhline(y=line_lat, color='grey', linestyle='--', linewidth=1)


def draw_circles_matrix(
        row,
        grid_width_on_img,
        grid_height_on_img,
        max_speeding_percentage,
        ax):
    # Translate grid coordinates to pixel coordinates on the image
    pixel_x = (row['grid_lon'] * grid_width_on_img)
    pixel_x += grid_width_on_img / 2

    pixel_y = (row['grid_lat'] * grid_height_on_img)
    pixel_y += grid_height_on_img / 2

    speeding_rate = row['speeding_percentage']
    speeding_rate /= max_speeding_percentage

    # Determine the color based on the speeding percentage
    color = plt.cm.RdYlGn(1 - speeding_rate)

    # Plot the point
    ax.scatter(pixel_x, pixel_y, color=color, s=100)


def get_grid_sizes(cols_cnt, rows_cnt):
    return const.IMG_WIDTH / cols_cnt, const.IMG_HEIGHT / rows_cnt


def output_speedings(speeding_map_png_path):
    plt.savefig(speeding_map_png_path, bbox_inches='tight', pad_inches=0)

    print('Speeding map saved to', speeding_map_png_path)

    plt.show()


def visualize_speedings(
        grid_summary,
        speeding_map_png_path,
        cols_cnt,
        rows_cnt):
    grid_width_on_img, grid_height_on_img = get_grid_sizes(cols_cnt, rows_cnt)

    map_img = mpimg.imread(const.MAP_PATH)  # Load the map image

    _, ax = plt.subplots()  # Set up the plot
    plt.axis('off')  # To turn off the axis labels and ticks

    # Ensure the figure fills the whole plot area
    plt.subplots_adjust(left=0, right=1, top=1, bottom=0)
    ax.imshow(map_img, extent=[0, const.IMG_WIDTH, 0, const.IMG_HEIGHT])

    remove_axes_tick_marks(ax)
    draw_grid(cols_cnt, rows_cnt, ax, grid_width_on_img, grid_height_on_img)

    max_speeding_percentage = grid_summary['speeding_percentage'].max()

    if max_speeding_percentage != 0:
        for _, row in grid_summary.iterrows():
            draw_circles_matrix(
                row,
                grid_width_on_img, grid_height_on_img,
                max_speeding_percentage,
                ax
            )

    output_speedings(speeding_map_png_path)


def visualize_speed(
        df,
        speed_dist_png_path,
        grid_summary,
        speeding_map_png_path,
        cols_cnt,
        rows_cnt):
    visualize_speed_dist(df, speed_dist_png_path)
    visualize_speedings(
        grid_summary,
        speeding_map_png_path,
        cols_cnt,
        rows_cnt
    )


def plot_buses_locs_around_point(df, point_coords, point_name, png_path):
    plt.figure(figsize=(10, 6))
    plt.scatter(
        df['Lon'], df['Lat'],
        alpha=0.5,
        c='blue',
        marker='o',
        label='Buses'
    )

    point_latitude, point_longitude = point_coords

    plt.scatter(
        x=point_longitude, y=point_latitude,
        color='red',
        marker='x',
        label=point_name
    )
    plt.title('Buses localizations in relation to ' + point_name)
    plt.xlabel('Longitude')
    plt.ylabel('Latitude')
    plt.legend()
    plt.grid(True)

    # Set x and y axis limits here
    plt.xlim(const.INITIAL_MIN_LON, const.INITIAL_MAX_LON)
    plt.ylim(const.INITIAL_MIN_LAT, const.INITIAL_MAX_LAT)

    # Save the figure before plt.show()
    plt.savefig(png_path, bbox_inches='tight', pad_inches=0)
    print('Buses distribution saved to', png_path)

    # Show the plot
    plt.show()
